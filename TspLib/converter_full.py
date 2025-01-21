def read_atsp_to_adjacency_matrix(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    dimension = 0
    edge_weight_section_start = False
    matrix = []
    i =0
    for line in lines:
        line = line.strip()
        if line.startswith("DIMENSION"):
            dimension = int(line.split(":")[1].strip())
        elif line.startswith("EDGE_WEIGHT_SECTION"):
            edge_weight_section_start = True
        elif edge_weight_section_start:
            if line == "EOF":
                break
            row = list(map(int, line.split()))
            row =[-1 if value == 100000000 or value == 9999999 else value for value in row]
            matrix.extend(row)
    
    # Transform the flat list into a square matrix
    adjacency_matrix = []
    for i in range(dimension):
        adjacency_matrix.append(matrix[i * dimension:(i + 1) * dimension])
    
    return adjacency_matrix

# Zapis macierzy do pliku w czytelnej formie
def save_adjacency_matrix(matrix, output_path):
    with open(output_path, 'w') as file:
        for row in matrix:
            file.write("  ".join(map(str, row)) + "\n")

# Przykład użycia
input_file = "D:\\Studia\\PEA\\PEA\\TspLib\\ASYM\\kro124p.atsp"
output_file = "D:\\Studia\\PEA\\PEA\\build\\Dane\\ATSP\\kro124p.txt"

adjacency_matrix = read_atsp_to_adjacency_matrix(input_file)
save_adjacency_matrix(adjacency_matrix, output_file)

print("Macierz sąsiedztwa została zapisana w pliku:", output_file)