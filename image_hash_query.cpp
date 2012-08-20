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
#include "pHash.h"

extern "C" {
#include "mvptree.h"
}

void print_usage()
{
    printf("\nUsage: image_hash_query -f mvp_filename [-h image_hash] [-i image_filename] -r radius\n\n");
    printf("  mvp_filename      - file from which to read the tree\n");
    printf("  image_hash        - hash of image to find duplicates of\n");
    printf("  image_filename    - filename of image to find duplicates of\n");
    printf("  radius            - radius for query operation\n");
    printf("\n");
}

#define MVP_BRANCHFACTOR 2
#define MVP_PATHLENGTH   5 
#define MVP_LEAFCAP      25 

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
    char *image_filename = NULL;
    ulong64 hashvalue = 0;
    float radius = 21;
    const int knearest = 5;
    
    while ((option = getopt (argc, argv, "f:h:i:r:")) != -1)
    {
        switch (option)
        {
        case 'f':
            mvp_filename = optarg;
            break;
        case 'h':
            sscanf(optarg, "%llx", &hashvalue);
            break;
        case 'i':
            image_filename = optarg;
            break;              
        case 'r':
            radius = atof(optarg);
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
    
    if (mvp_filename == NULL || (hashvalue == 0 && image_filename == NULL))
    {
        print_usage();
        return -1;
    }

    if (image_filename != NULL)
    {
        const char *name = strrchr(image_filename, '/') + 1;  
        if (ph_dct_imagehash(image_filename, hashvalue) < 0)
        {
            return -1;
        }
    }

    MVPDP *points = dp_alloc(MVP_UINT64ARRAY);
    points->id = strdup("0");
    points->data = malloc(1*UINT64ARRAY);
    points->datalen = 1;
    memcpy(points->data, &hashvalue, UINT64ARRAY);
    
    MVPError err;
    CmpFunc distance_func = hamming_distance_cb;
    MVPTree *tree = mvptree_read(mvp_filename, distance_func, MVP_BRANCHFACTOR, MVP_PATHLENGTH,\
                                                                         MVP_LEAFCAP, &err);
    assert(tree);
    
    unsigned int nbresults;
    MVPDP **results = mvptree_retrieve(tree, points, knearest,\
                                                           radius, &nbresults, &err);
    if (nbresults > 0)
    {
        for (int j=0 ; j != nbresults; ++j)
        {
            printf("%s", results[j]->id);
            if (j < (nbresults-1))
                printf(",");
        }
    }
    
    free(results);
    mvptree_clear(tree, free);

    return 0;
}
