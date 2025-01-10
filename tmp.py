import random

# Parametry algorytmu
n_miast = 5
n_mrowek = 5
n_iteracji = 10

# Tworzenie przykładowej macierzy odległości między miastami
distances = [[0 if i == j else random.randint(1, 10) for j in range(n_miast)] for i in range(n_miast)]

# Funkcja wybierająca następne miasto dla mrówki
def wybierz_miasto(pheromone_levels, current_city, visited):
    probs = []
    for i in range(n_miast):
        if i not in visited:
            # Prawdopodobieństwo jest proporcjonalne do poziomu feromonów
            prob = pheromone_levels[current_city][i]
            probs.append((i, prob))
    return max(probs, key=lambda x: x[1])[0] if probs else -1

# Symulacja algorytmu mrówkowego
def ant_algorithm():
    # Inicjalizacja poziomów feromonów
    pheromone_levels = [[1 for _ in range(n_miast)] for _ in range(n_miast)]

    for _ in range(n_iteracji):
        all_paths = []
        for _ in range(n_mrowek):
            current_city = random.randint(0, n_miast - 1)
            visited = [current_city]
            path = [current_city]

            while len(visited) < n_miast:
                next_city = wybierz_miasto(pheromone_levels, current_city, visited)
                if next_city == -1:
                    break
                visited.append(next_city)
                path.append(next_city)
                current_city = next_city

            all_paths.append(path)

        # Aktualizacja poziomu feromonów
        for path in all_paths:
            for i in range(len(path) - 1):
                pheromone_levels[path[i]][path[i+1]] += 1 / distances[path[i]][path[i+1]]

    return all_paths

# Uruchomienie algorytmu
paths = ant_algorithm()
print("Znalezione ścieżki: ", paths)