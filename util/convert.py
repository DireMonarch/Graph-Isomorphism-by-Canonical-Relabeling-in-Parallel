import sys

def graph_to_graph6(G):
    ## https://users.cecs.anu.edu.au/~bdm/data/formats.txt
    output = chr(len(G[0])+63)
    # print(output)
    bits = []
    for col in range(len(G[0]))[1:]:
        for row in range(col):
            bits.append(str(G[row][col]))
            # print(col,row, '=', G[row][col])

    for _ in range(6-(len(bits) % 6) ):
        bits.append('0')
    
    for i in range(int(len(bits) / 6)):
        strval = ''.join(bits[i*6:i*6+6])
        output += chr(int(strval,2)+63)
    return output
    
    
def process_input_file(infilename, outfilename, limit=None):
    ##
    #
    #  https://mikhad.github.io/graph-builder/#2023
    #
    ##
    
    edges = []
    # vertices = []
    separator = ' '
    with open(infilename, 'r') as infile:
        while(infile.readable()):
            edge = infile.readline()
            if len(edge) == 0: break

            edge = edge.split(separator)
            edges.append(edge[0:2])
    
    am = []
    cross = {}
    next = 0
    for edge in edges:
        if edge[0] not in cross:
            cross[edge[0]] = next
            next += 1
        if edge[1] not in cross:
            cross[edge[1]] = next
            next += 1
        edge[0] = cross[edge[0]]
        edge[1] = cross[edge[1]]

    print(f'Vertices: {next}    Edges: {len(edges)}')
    
    for _ in range(next):
        am.append([0 for _ in range(next)])

    for edge in edges:
        am[edge[0]][edge[1]] = 1
        am[edge[1]][edge[0]] = 1

    if limit is not None:
        am2 = []
        for i in range(limit):
            am2.append(am[i][:limit])
        am = am2
                
    print(f'Output Vertices: {len(am)}')

    g6 = graph_to_graph6(am)
    with open(outfilename, 'w') as outfile:
        outfile.write(g6)


def usage():
    print('Usage: ')
    print(f'{sys.argv[0]} <options> inputfilename outputfilename')
    print('\tinputfilename - file to use for the input edges')
    print('\toutputfilename - file to use for the output graph')


def main():
    if len(sys.argv) < 3:
        usage()
        exit(0)

    infilename = sys.argv[1]
    outfilename = sys.argv[2]

    if len(sys.argv)>3:
        process_input_file(infilename, outfilename, int(sys.argv[3]))
    else:
        process_input_file(infilename, outfilename)

if __name__ == '__main__':
    main()