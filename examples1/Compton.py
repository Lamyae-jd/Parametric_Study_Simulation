import ROOT

# Variable pour stocker le nombre total d'événements qui ont créé de la diaphonie optique
total_event = 0
total_diaphonie = 0
total_compton = 0

# Ouvrir le fichier de sortie de Gate en mode lecture
input_file = ROOT.TFile.Open("Simulation 4/outputCrystal2.root", "READ")

# Accéder à l'arbre Singles
singles_tree = input_file.Get("Singles")

# Créer un dictionnaire pour stocker les photons détectés pour chaque événement
photons = {}

# Parcourir l'arbre Singles pour extraire les informations sur les photons détectés pour chaque événement
for entry in singles_tree:
   
    event_id = entry.eventID
    detector_id = entry.crystalID
    time = entry.time
    
    # Vérifier si l'événement est déjà présent dans le dictionnaire
    if event_id not in photons:
        photons[event_id] = {}
    
    # Ajouter l'identifiant du scintillateur et le temps de détection à la liste des photons détectés pour cet événement
    if detector_id not in photons[event_id]:
        photons[event_id][detector_id] = []
    photons[event_id][detector_id].append(time)
    
    total_event += 1

# Fermer le fichier d'entrée
input_file.Close()

# Analyse des événements avec des photons détectés dans au moins deux scintillateurs différents
for event_id, detector_times in photons.items():
    detector_ids = list(detector_times.keys())
    if len(detector_ids) >= 2:
        print(f"Event {event_id} : {detector_ids}")
        total_diaphonie += 1
        
        # Vérifier si les temps de détection sont compatibles avec le processus de Compton
        for i in range(len(detector_ids)):
            for j in range(i+1, len(detector_ids)):
                id1 = detector_ids[i]
                id2 = detector_ids[j]
                times1 = detector_times[id1]
                times2 = detector_times[id2]
                
                # Calculer la différence de temps entre les deux photons
                for t1 in times1:
                  
                    for t2 in times2:
                        delta_t = abs(t1 - t2)
                        
                        # Vérifier si la différence de temps est compatible avec le processus de Compton
                        if delta_t > 5 and delta_t < 1000:
                            print(f"  Compton: {id1}-{id2}, delta_t={delta_t:.1f}")
                            total_compton += 1

print(f"Total des événements: {total_event}")
print(f"Total des événements avec diaphonie optique: {total_diaphonie}")
print(f"Total des événements avec diaphonie optique due au Compton: {total_compton}")
