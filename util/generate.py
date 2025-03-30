import sys
import random

##
#
#  https://terpai.web.elte.hu/d6visual.html
#    Website to visualze G6 string as graph
#
##


def graph_to_graph6(G):
    ## https://users.cecs.anu.edu.au/~bdm/data/formats.txt
    n = len(G[0])
    if n <= 62:
        output = chr(n+63)
    elif n <= 258047:
        ## byte 1
        output = chr(126)
        
        ## byte 2 - 4
        for i in range(2,-1,-1):
            next = (n >> (6 * i) & 0b111111) + 63
            # print(f'i: {i}    next:{next}    chr:{chr(next)}')
            output += chr(next)
        
    elif n <= 68719476735:
        ## byte 1
        output = chr(126)
        ## byte 2
        output += chr(126)
        
        ## byte 3 - 8
        for i in range(5,-1,-1):
            next = (n >> (6 * i) & 0b111111) + 63
            output += chr(next)
    else:
        print(f'Sorry, this file format only works up to graphs with 68719476735 or fewer nodes!')
        exit(1)

    # print(f'Header: {output}')

    bits = []
    rowbits = []
    for col in range(len(G[0]))[1:]:
        rowbits = []
        # print(col,'',end=' ' )
        for row in range(col):
            bits.append(str(G[row][col]))
            
            # print(col,row, '=', G[row][col])
        #     print(G[row][col], end=' ')
        # print('')
            

    topad = 0 if (len(bits)%6 == 0) else 6-(len(bits) % 6)
    # print(f'Bits:  {len(bits)}   Pad: {topad}   Bytes: {(len(bits) + topad)/6}')
    for _ in range(topad):
        bits.append('0')
    
    for i in range(int(len(bits) / 6)):
        strval = ''.join(bits[i*6:i*6+6])
        output += chr(int(strval,2)+63)
    return output
    
    
def generate(v, e, md, filename):
    am = []
    for _ in range(v):
        am.append([0 for _ in range(v)])
        
    edge_ct = 0
    while (edge_ct < e):
        a = random.randint(0, v-1)
        b = random.randint(0, v-1)
        if a != b:
            if am[a][b] == 0 and sum(am[a]) < md:
                am[a][b] = 1
                am[b][a] = 1
                edge_ct += 1
    
    # for r in am:
    #     print(r)
        
    with open(f'{filename}-{v}-{e}-{md}.g6', 'w') as outfile:
        outfile.write(graph_to_graph6(am))

    print(f'Graph written to {filename}-{v}-{e}-{md}.g6')

def usage():
    print('Usage: ')
    print(f'{sys.argv[0]} outputfilename  vertices  edges max_degree')


def main():
    if len(sys.argv) < 5:
        usage()
        exit(0)

    outfilename = sys.argv[1]
    vertices = int(sys.argv[2])
    edges = int(sys.argv[3])
    max_degree = int(sys.argv[4])

    generate(vertices, edges, max_degree, outfilename);

if __name__ == '__main__':
    main()