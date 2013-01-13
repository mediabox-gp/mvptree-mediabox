#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "mb_common.h"

extern "C" {
#include "mvptree.h"
}

void print_usage()
{
    printf("\nUsage: image_hash_print -f mvp_filename\n\n");
    printf("  mvp_filename      - file from which to read the tree\n");
    printf("\n");
}


float hamming_distance_cb(MVPDP *pointA, MVPDP *pointB)
{
    if (!pointA || !pointB || pointA->datalen != pointB->datalen)
    {
        return -1.0f;
    }

    uint64_t a = *((uint64_t*)pointA->data);
    uint64_t b = *((uint64_t*)pointB->data);

    int res = ph_hamming_distance(a, b);

    return (float)res;
}


int main(int argc, char **argv)
{
    int option = 0;
    char *mvp_filename = NULL;

    while ((option = getopt (argc, argv, "f:")) != -1)
    {
        switch (option)
        {
        case 'f':
            mvp_filename = optarg;
            break;
        case '?':
            if (optopt == 'f' || optopt == 'h' || optopt == 'i' || optopt == 'r')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
            return -1;
        default:
            print_usage();
            return -1;
        }
    }

    if (mvp_filename == NULL)
    {
        print_usage();
        return -1;
    }

    MVPError err;
    CmpFunc distance_func = hamming_distance_cb;
    MVPTree *tree = mvptree_read(mvp_filename, distance_func, MVP_BRANCHFACTOR, MVP_PATHLENGTH,\
                                                                         MVP_LEAFCAP, &err);
    assert(tree);

    printf("-----------------------print-------------------------\n");
    mvptree_print(stdout,tree);
    printf("-----------------------------------------------------\n\n");

    mvptree_clear(tree, free);

    return 0;
}
