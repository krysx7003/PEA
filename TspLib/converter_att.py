import math

linelist = []
vertlist = []
matrix = []

def read_from_file():
    with open("D:\\Studia\\PEA\\PEA\\TspLib\\SYM\\pr152.tsp", "r") as f:
        for line_number, line in enumerate(f):
            linelist.append((line_number, line.strip()))

def create_vert_list():
    for i in range(48):
        sLine = linelist[i+6][1].split(" ",3)
        x = float(sLine[1])
        y = float(sLine[2])
        vertlist.append((x,y))

def calculate_weight(start,end):
    return round(math.sqrt((pow((vertlist[start][0] - vertlist[end][0]),2) + pow((vertlist[start][0] - vertlist[end][0]),2))/10))

def convert_att():
    create_vert_list()
    for i in range (0,48):
        row = []
        for j in range (0,47):
            if(j==i):
                row.append(0)
            else:
                row.append(calculate_weight(i,j))
        matrix.append(row)

def save_to_file(sFileName):
    sFileName += ".txt"
    with open(sFileName, "w") as f:
        for row in matrix:
            line = ' '.join(map(str, row))
            f.write(line + "\n")
        
def main():
    read_from_file()
    sFileName = linelist[0][1].split(": ")[1]
    convert_att()
    save_to_file(sFileName)
if __name__ == '__main__':
    main() 