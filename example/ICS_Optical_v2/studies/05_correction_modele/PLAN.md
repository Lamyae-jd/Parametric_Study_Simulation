# Étude 05 — Correction ICS par le modèle (small_v2) sur le détecteur de l'étude paramétrique

Plan validé avec l'utilisatrice le 2026-10-18 (« go ahead »). Branche `study05-correction-modele`.
Le code de l'étude (`analysis/`) est **réutilisé sans modification** (import en lecture seule).

## Question
Les études 01–04 montrent que l'ICS (avec fluorescence K de Lu/Gd) crée des coups en trop, déplace des coups et
fait perdre de la MTF, de la DQE et du CNR. **Combien de cette perte une correction par le modèle récupère-t-elle ?**

## Données
- Détecteurs : LGSO 1,2 mm (pas 1,4 mm) et LGSO 2,0 mm (pas 2,2 mm), PTFE 0,2 mm, longueur 2,5 mm, réseau 19×19,
  faisceau sur le cristal central (point d'entrée uniforme dans sa cellule).
- Paire optique de référence de l'étude : PTFE poli (1,2 mm : `b8cacfd56f78`, 2,0 mm : `89946b8e94b4`).
- **Test** : les 16 runs avec spectre CT (80/100/120/140 kVp, direct et 20 cm d'eau) — jamais vus à l'entraînement.
- **Entraînement / validation du modèle réentraîné** : les runs mono-énergie 20–160 keV des deux détecteurs
  (balayages G1_energy, G1B_energy) ; validation = 15 % des événements de chaque run, tirés au hasard.
- Comme dans `performance.py` : 300 000 premiers événements avec dépôt par run (MAX_EVENTS).

## Lecture (identique à la variante `ics` de l'étude)
- Énergie de chaque cristal : photoélectrons Poisson, lumière confinée au pixel, étalonnage sur le photopic
  (`performance.pixel_signals("ics")`).
- Temps de chaque cristal : temps du 1er photoélectron (dépôt + émission LGSO + transport GATE + SPTR 100 ps),
  `performance._timestamps` (le modèle de temps de l'étude).
- Seuil par hit : **5 keV** (système de l'utilisatrice) et **20 keV** (seuil principal de l'étude).
- Un événement = un photon (pas d'empilement), comme dans toute l'étude.

## Échantillon pour le modèle (comme small_v2)
- Fenêtre 5×5 cristaux centrée sur le hit le plus énergétique ; cellule = ligne·5 + colonne (ligne = X, colonne = Y).
- x = [E (keV), T − t_min (ns), actif] ; pixels vides : T − t_min = −t_min (convention d'origine).
- Vrai premier cristal = dépôt le plus précoce (temps GATE vrai), avant seuil. Vérifié contre l'étiquette
  « direct / pénétration » de `reduce.py`.
- Pool ICS = événements avec dépôt dans ≥ 2 cristaux et ≥ 1 hit visible. Cible = le hit visible de la fenêtre
  au dépôt le plus précoce. Métriques sur tout le pool : AUPRC, précision, rappel, F1, AccTop1.

## Variantes comparées (mêmes photoélectrons pour brut et corrigé : comparaison appariée)
| variante | ce qu'elle compte |
|---|---|
| `ideal` | pas d'ICS (variante de l'étude) — plafond physique |
| `ics` (brut) | un coup par cristal au-dessus du seuil (variante de l'étude) |
| `oracle` | un coup par photon sur le vrai premier cristal visible — meilleure correction possible |
| `small_v2` tel quel | un coup par photon sur le choix du modèle (argmax parmi les hits actifs) |
| `small_v2` réentraîné | idem, modèle réentraîné sur les runs mono-énergie (avec fluorescence) |
Correction : un photon à 1 hit → ce hit ; ≥ 2 hits → choix du modèle ; énergie du coup = somme des hits de la
fenêtre. Les hits visibles **hors fenêtre** restent des coups séparés (ce que ferait le matériel).

## Métriques (fonctions de l'étude, `image_metrics.py`, `timing.py`, `image_sim.py`)
- Comptage : coups en trop par photon détecté, photons mal placés (coup ≠ cristal irradié), efficacité.
- MTF présamplée (fN/2, fN), DQE(0), SNR relatif, CNR relatif (disques 1, 2, 5 mm), à 5 et 20 keV.
- Images simulées d'un fantôme à disques de faible contraste (`image_sim`), même photons pour toutes les variantes.
- Énergie : spectre des coups (part des coups < 30 keV) ; temps : FWHM des horodatages des coups.

## Vérifications avant résultats
- V1 : mes variantes `ideal` / `ics` redonnent les chiffres de `results/performance.json` (à la statistique près).
- V2 : vrai premier cristal = cristal « direct / pénétration » de `reduce.py` ; 1 seul par événement.
- V3 : canal temps — le modèle tel quel est aussi testé avec le temps GATE idéal (sans bruit) pour mesurer
  ce que le temps réaliste lui enlève (les écarts vrais entre cristaux sont de quelques ps).

## Livrables
`results/RESULTS.md`, `results/metrics.json`, figures (MTF, images, spectres) dans `figures/`.
Données intermédiaires dans `data/` (non versionnées).
