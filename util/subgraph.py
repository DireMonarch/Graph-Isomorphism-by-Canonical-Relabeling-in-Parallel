import sys


def process_input_file(infilename, outfilename, start, limit):
    edges = []
    # vertices = []
    separator = None
    with open(infilename, 'r') as infile:
        while(infile.readable()):
            edge = infile.readline()
            if len(edge) == 0: break
            if separator is None:
                if ',' in edge:
                    separator = ','
                elif ' ' in edge:
                    separator = ' '
                else:
                    print('Can\'t determin separator character!', edge)
                    exit(1)
            edge = edge.split(separator)
            if len(edge) != 2:
                print('Invalid edge found: ', edge)
                exit(1)
            edge[0] = int(edge[0])
            edge[1] = int(edge[1])
            # if edge[0] >= start and edge[0] < start+limit and ((edge[1] >= start and edge[1] < start+limit) or (len(vertices) < limit)):
            if (edge[0] >= start and edge[0] < start+limit) or (edge[1] >= start and edge[1] < start+limit):
                edges.append(edge)

    
    print(edges[:20])

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

    print(edges[:20])

def usage():
    print('Usage: ')
    print(f'{sys.argv[0]} <options> inputfilename outputfilename')
    print('\tinputfilename - file to use for the input edges')
    print('\toutputfilename - file to use for the output graph')
    print('\nOptions:')
    print('\t--limit start length - limit graph to a specific length (number of vertices) and starting vertex number')

def main():
    if len(sys.argv) < 3:
        usage()

    infilename = sys.argv[-2]
    outfilename = sys.argv[-1]

    if '--limit' in sys.argv and len(sys.argv) > 5:
        idx = sys.argv.index('--limit')
        start = int(sys.argv[idx+1])
        limit = int(sys.argv[idx+2])
        process_input_file(infilename, outfilename, start, limit)

if __name__ == '__main__':
    main()