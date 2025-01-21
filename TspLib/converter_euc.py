import numpy as np

def euclidean_distance(p1, p2):
    return round(((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2) ** 0.5)

def read_tsp_file(filename):
    nodes = {}
    with open(filename, 'r') as file:
        lines = file.readlines()
        start_reading = False
        for line in lines:
            if line.strip() == "NODE_COORD_SECTION":
                start_reading = True
                continue
            if start_reading:
                if line.strip() == "EOF":
                    break
                parts = line.split()
                if len(parts) == 3:
                    node_id = int(parts[0])
                    x, y = int(parts[1]), int(parts[2])
                    nodes[node_id] = (x, y)
    return nodes

def generate_adjacency_matrix(nodes):
    size = len(nodes)
    adjacency_matrix = np.zeros((size, size), dtype=int)
    for i in range(1, size + 1):
        for j in range(1, size + 1):
            if i != j:
                adjacency_matrix[i-1, j-1] = euclidean_distance(nodes[i], nodes[j])
    return adjacency_matrix

def save_matrix_to_file(matrix, output_filename):
    np.savetxt(output_filename, matrix, fmt='%d')

def main():
    input_filename = "D:\\Studia\\PEA\\PEA\\TspLib\\SYM\\lin318.tsp"  # Plik wejściowy w formacie TSP
    output_filename = "lin318.txt"  # Plik wyjściowy
    nodes = read_tsp_file(input_filename)
    adjacency_matrix = generate_adjacency_matrix(nodes)
    save_matrix_to_file(adjacency_matrix, output_filename)
    print(f"Macierz sąsiedztwa zapisana do {output_filename}")

if __name__ == "__main__":
    main()