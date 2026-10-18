# Résultats — étude 05 : correction ICS par le modèle (2026-10-18)

Plan : `../PLAN.md`. Chiffres : `metrics.json` (par run), `summary.json` (tableaux ci-dessous). Figures : `../figures/`.
Scripts : `../scripts/` (`check_readout.py` → `evaluate.py` → `aggregate.py` ; `make_trainset.py` → `train.py`).

## En bref
- Sur ce détecteur LGSO **avec fluorescence K** (absente des runs ICRP), l'ICS a un vrai coût image. À 5 keV, en
  1,2 mm sous 120 kVp + 20 cm d'eau, il crée **+11 % de coups en trop** et place **10 % des photons sur le mauvais
  cristal**. Il fait perdre **4 % de MTF** à fN/2, **8 % de DQE(0)** et **3–5 % de CNR** sur un disque de 1 mm.
- **small_v2 réentraîné** (2 338 paramètres, même architecture que la version FPGA), en moyenne sur les 8 spectres
  CT à 5 keV :
  - il supprime **99,7 % des coups en trop** et récupère **toute la DQE(0)** ;
  - il récupère **77–81 % de la perte de MTF** et **40–51 % de la perte de CNR (disque de 1 mm)** ;
  - la correction parfaite (oracle) fait 84–87 % pour la MTF et 65–69 % pour le CNR.
- **Le choix du cristal compte.** Regrouper les hits d'un photon sur le cristal le plus énergétique, sans modèle,
  supprime aussi les coups en trop, mais rend le CNR des petits disques **pire que sans correction**. Avec la
  fluorescence, le cristal le plus énergétique est souvent celui où le rayon X K est absorbé, pas le premier
  cristal touché.
- small_v2 **tel quel** (entraîné sans fluorescence) fait déjà 67–73 % sur la MTF. Le réentraînement apporte le reste.

## Données et méthode
- Détecteurs de l'étude : LGSO 1,2 mm (pas 1,4 mm) et 2,0 mm (pas 2,2 mm), PTFE, réseau 19×19, faisceau sur le
  cristal central. Lecture = variante `ics` de `performance.py` (photoélectrons Poisson, lumière confinée,
  étalonnage au photopic). Horodatage = 1er photoélectron (émission LGSO, transport GATE, SPTR 100 ps).
- Test : 16 runs à spectre CT (80–140 kVp, direct et 20 cm d'eau), 300 000 photons détectés par run. Ces runs ne
  sont jamais vus à l'entraînement.
- Réentraînement : 30 runs mono-énergie 20–160 keV, aux seuils 5 et 20 keV. 6 M photons pour l'entraînement et
  1,2 M pour la validation (15 % des événements de chaque run). Même recette que small_v2.
  Meilleure AUPRC de validation 0,882, à l'époque 59 : arrêt au plafond de 60 époques, le score montait encore un peu.
- Correction : un photon à 1 hit → ce hit ; ≥ 2 hits → le choix du modèle parmi les hits de la fenêtre 5×5. Le photon
  donne **un** coup, d'énergie = somme de ses hits. Les hits hors fenêtre restent des coups séparés (< 0,04 %).
- Toutes les variantes utilisent **les mêmes photoélectrons**, donc la comparaison est appariée.

## Vérifications
- V1 : mes variantes `ideal` et `ics` redonnent `results/performance.json`. Comptage à ≤ 0,1 % sur les 16 runs ;
  MTF, DQE et CNR identiques à 1e-4 (MTF(fN/2) 0,8996 contre 0,8996).
- V2 : le cristal du dépôt le plus précoce est toujours le cristal « direct / pénétration » de `reduce.py`.
  Les 0,2–0,6 % de photons sans cristal primaire sont ceux dont le primaire n'a rien déposé dans un cristal.
- V3 (`logs/check_time_channel.json`) : **small_v2 tel quel n'utilise pas le canal temps.** Remplacer le temps
  mesuré par le temps GATE idéal, ou par 0, change **0** décision sur 330 718 photons ICS à ≥ 2 hits. Le réentraîné
  change 0,3 % de ses décisions. Les vrais écarts de temps entre cristaux sont de quelques ps ; l'horodatage a
  ~100 ps de bruit.

## Modèles — pool ICS entier des 16 runs de test
| détecteur, seuil | modèle | AUPRC | Précision | Rappel | F1 | AccTop1 |
|---|---|---|---|---|---|---|
| 1,2 mm, 5 keV (211 355 ICS) | tel quel | 0,825 | 0,789 | 0,744 | 0,766 | 0,796 |
| | réentraîné | **0,945** | 0,860 | 0,879 | 0,870 | 0,896 |
| 2,0 mm, 5 keV (135 658 ICS) | tel quel | 0,830 | 0,789 | 0,758 | 0,773 | 0,800 |
| | réentraîné | **0,949** | 0,861 | 0,886 | 0,874 | 0,897 |
| 1,2 mm, 20 keV (209 737 ICS) | tel quel | 0,682 | 0,661 | 0,996 | 0,794 | 0,828 |
| | réentraîné | **0,836** | 0,740 | 0,939 | 0,828 | 0,923 |
| 2,0 mm, 20 keV (134 690 ICS) | tel quel | 0,682 | 0,661 | 0,996 | 0,794 | 0,829 |
| | réentraîné | **0,838** | 0,739 | 0,942 | 0,828 | 0,923 |

La précision et le rappel sont pris au seuil du meilleur F1. La tâche est plus dure que sur les runs ICRP
(AUPRC 0,998), car la fluorescence crée des motifs à 2 cristaux ambigus. À 20 keV, 35 % des premiers cristaux sont
sous le seuil, donc invisibles pour toute méthode.

## Impact image — run de référence 120 kVp + 20 cm d'eau, seuil 5 keV
CNR relatif à l'idéal : « DQE » = prédiction de l'étude (observateur idéal) ; « images » = mesuré sur le fantôme à
disques simulé (± 0,6–0,9 % statistique).

**LGSO 1,2 mm**
| | coups en trop | mal placés | MTF fN/2 | MTF fN | DQE(0) | CNR 1 mm (DQE) | CNR 1 mm (images) | coups < 30 keV | temps FWHM |
|---|---|---|---|---|---|---|---|---|---|
| sans ICS (idéal) | 0 | 0 | 0,924 | 0,718 | 0,740 | 1 | 1 | 9,0 % | 306 ps |
| brut (avec ICS) | 11,3 % | 9,8 % | 0,886 | 0,633 | 0,682 | 0,973 | 0,950 | 13,9 % | 319 ps |
| regroupé, cristal le + énergétique | 0,04 % | 9,8 % | 0,894 | 0,655 | 0,740 | 0,951 | 0,958 | 9,0 % | 312 ps |
| small_v2 tel quel | 0,04 % | 4,8 % | 0,910 | 0,688 | 0,740 | 0,978 | 0,980 | 9,0 % | 315 ps |
| **small_v2 réentraîné** | 0,04 % | 3,4 % | 0,915 | 0,699 | 0,740 | 0,985 | 0,987 | 9,0 % | 314 ps |
| correction parfaite (oracle) | 0,04 % | 1,8 % | 0,920 | 0,710 | 0,740 | 0,994 | 0,994 | 9,0 % | 314 ps |

**LGSO 2,0 mm**
| | coups en trop | mal placés | MTF fN/2 | MTF fN | DQE(0) | CNR 1 mm (DQE) | CNR 1 mm (images) | coups < 30 keV | temps FWHM |
|---|---|---|---|---|---|---|---|---|---|
| sans ICS (idéal) | 0 | 0 | 0,916 | 0,690 | 0,826 | 1 | 1 | 8,6 % | 300 ps |
| brut (avec ICS) | 7,2 % | 6,2 % | 0,899 | 0,645 | 0,780 | 0,991 | 0,975 | 11,8 % | 310 ps |
| regroupé, cristal le + énergétique | 0,01 % | 6,2 % | 0,905 | 0,661 | 0,826 | 0,978 | 0,976 | 8,6 % | 304 ps |
| small_v2 tel quel | 0,01 % | 3,0 % | 0,911 | 0,677 | 0,826 | 0,990 | 0,990 | 8,6 % | 307 ps |
| **small_v2 réentraîné** | 0,01 % | 2,0 % | 0,913 | 0,682 | 0,826 | 0,994 | 0,993 | 8,6 % | 307 ps |
| correction parfaite (oracle) | 0,01 % | 1,0 % | 0,915 | 0,686 | 0,826 | 0,998 | 0,998 | 8,6 % | 306 ps |

« Mal placés » = le coup du photon n'est pas sur le cristal irradié. Même la correction parfaite en garde 1–2 % :
le primaire est entré par le côté, ou son premier dépôt est sous le seuil.

## Part de la perte ICS récupérée — moyenne des 8 spectres CT
(valeur corrigée − brut) / (idéal − brut). Figure `../figures/recovered_share.png`.

| détecteur, seuil | variante | coups en trop | mal placés | MTF fN/2 | MTF fN | DQE(0) | CNR 1 mm | CNR 2 mm |
|---|---|---|---|---|---|---|---|---|
| 1,2 mm, 5 keV | regroupé (max E) | 100 % | 0 % | 14 % | 18 % | 100 % | **−81 %** | −47 % |
| | tel quel | 100 % | 57 % | 67 % | 69 % | 100 % | 33 % | 46 % |
| | **réentraîné** | 100 % | 67 % | **77 %** | 78 % | 100 % | **51 %** | 60 % |
| | oracle | 100 % | 77 % | 84 % | 86 % | 100 % | 69 % | 75 % |
| 2,0 mm, 5 keV | regroupé (max E) | 100 % | 0 % | 24 % | 30 % | 100 % | **−144 %** | −108 % |
| | tel quel | 100 % | 59 % | 73 % | 75 % | 100 % | 16 % | 29 % |
| | **réentraîné** | 100 % | 68 % | **81 %** | 82 % | 100 % | **40 %** | 49 % |
| | oracle | 100 % | 79 % | 87 % | 89 % | 100 % | 65 % | 70 % |
| 1,2 mm, 20 keV | réentraîné | 100 % | 28 % | 39 % | 40 % | 98 % | 16 % | 23 % |
| | oracle | 100 % | 36 % | 45 % | 47 % | 98 % | 26 % | 31 % |
| 2,0 mm, 20 keV | réentraîné | 100 % | 29 % | 43 % | 45 % | 99 % | 10 % | 14 % |
| | oracle | 100 % | 37 % | 49 % | 51 % | 99 % | 21 % | 24 % |

- La DQE(0) et les coups en trop sont réglés par le **regroupement** (un coup par photon), quel que soit le cristal
  choisi. La MTF et le CNR des petits détails dépendent du **choix du cristal** : c'est là que le modèle compte.
- Le seuil bas (5 keV) est un atout : à 20 keV, un tiers des premiers cristaux sont invisibles, et même l'oracle ne
  récupère que la moitié de la perte de MTF.

## Énergie et temps
- Spectre des coups (`../figures/spectra.png`) : l'ICS crée un faux pic vers 40–50 keV et une bosse vers 10–20 keV
  (coups < 30 keV : 13,9 % au lieu de 9,0 % en 1,2 mm). La correction, qui somme l'énergie du photon, rend le
  spectre idéal.
- Temps : FWHM des horodatages des coups 319 → 314 ps (idéal 306 ps) en 1,2 mm. Le gain est faible.

## Limites
- Un photon à la fois (pas d'empilement), pas de diaphonie optique ni de diaphonie SiPM : ce sont les hypothèses de
  la variante `ics` de l'étude.
- Le modèle réentraîné et le test viennent de la **même simulation de détecteur** (runs distincts, spectres
  différents). Le transfert vers le vrai détecteur n'est pas montré.
- Les runs LGSO ICRP 300×300 (reconstruction du 18/10) n'avaient **pas** la fluorescence (module de désexcitation
  atomique non activé dans GATE). C'est pourquoi l'impact image y était minuscule. À activer dans les prochaines
  simulations.
- Version FPGA du modèle réentraîné : pas encore exportée.
