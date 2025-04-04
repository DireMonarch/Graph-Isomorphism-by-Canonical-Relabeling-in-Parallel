import sys
import networkx as nx

##
#
#  https://terpai.web.elte.hu/d6visual.html
#    Website to visualze G6 string as graph
#
##



def generate_erdos_renyi(filename, vertices, probability):
    G = nx.erdos_renyi_graph(vertices, probability)
    nx.graph6.write_graph6(G, filename)
    
    print(G)
    print(f'Erdos Renyi graph generated with {vertices} vertices and {probability} probability.  written to {filename}.')
  

def usage():
    print('Usage: ')
    print(f'{sys.argv[0]} ER basefilename  vertices probability \n\tGenerates Erdos-Renyi graph')


def main():
    if len(sys.argv) < 5:
        usage()
        exit(0)

    generator = sys.argv[1]
    vertices = int(sys.argv[3])
    probability = float(sys.argv[4])
    filename = sys.argv[2]

    if generator == 'ER':
        generate_erdos_renyi(filename, vertices, probability)
    else:
        usage()

if __name__ == '__main__':
    main()