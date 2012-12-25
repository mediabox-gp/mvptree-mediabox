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
    printf("\nUsage: image_hash_add [-f mvp_filename] filelist\n\n");
    printf("  mvp_filename  - file for storing mvp tree\n");
    printf("  filelist      - space delimited list of files to process\n");
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

    while ((option = getopt (argc, argv, "f:")) != -1)
    {
        switch (option)
        {
        case 'f':
            mvp_filename = optarg;
            break;
        case '?':
            if (optopt == 'f')
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

    // retval legend: -1 = critical error, 0 = all ok, 1 = errors encountered (non critical)
    int retval = -1;
    int nbfiles = argc - optind;
    if (nbfiles < 1)
    {
        print_usage();
        return -1;
    }

    printf ("%d files\n", nbfiles);
    char **files = (char**)malloc(nbfiles*sizeof(*files));
    assert(files);
    for (int i = 0; i != nbfiles; ++i)
    {
        files[i] = strdup(argv[i+optind]);
    }

    MVPDP **points = (MVPDP**)malloc(nbfiles*sizeof(MVPDP*));
    assert(points);

    MVPError err;
    CmpFunc distance_func = hamming_distance_cb;

    MVPTree *tree = NULL;
    if (mvp_filename != NULL)
    {
        tree = mvptree_read(mvp_filename, distance_func, MVP_BRANCHFACTOR, MVP_PATHLENGTH,\
                                                                             MVP_LEAFCAP, &err);
        assert(tree);
    }

    int num_errors = 0;
    int count = 0;
    ulong64 hashvalue;
    for (int i=0; i != nbfiles; ++i)
    {
        char *name = strrchr(files[i],'/')+1;

        if (ph_dct_imagehash(files[i], hashvalue) < 0)
        {
            printf("Unable to get hash value (file: '%s' )\n", files[i]);
            num_errors++;
            continue;
        }
        printf("@,%016llx,%s\n", (unsigned long long)hashvalue, name);

        points[count] = dp_alloc(MVP_UINT64ARRAY);
        points[count]->id = strdup(name);
        points[count]->data = malloc(1*MVP_UINT64ARRAY);
        points[count]->datalen = 1;
        memcpy(points[count]->data, &hashvalue, MVP_UINT64ARRAY);
        count++;
    }

    if (num_errors > 0)
    {
        printf("%d errors encountered\n", num_errors);
    }

    if (count > 0)
    {
        if (tree != NULL)
        {
            MVPError error = mvptree_add(tree, points, count);
            if (error != MVP_SUCCESS)
            {
                printf("Unable to add hash values to tree.\n");
                goto cleanup;
            }

            printf("Saving file '%s' ...\n", mvp_filename);
            error = mvptree_write(tree, mvp_filename, 00755);
            if (error != MVP_SUCCESS)
            {
                printf("Unable to save file.\n");
                goto cleanup;
            }
        }
        printf("DONE\n\n");

        retval = (num_errors > 0) ? 1 : 0; // Update return value
    }

cleanup:
    if (tree != NULL)
        mvptree_clear(tree, free);

    for (int i=0; i != nbfiles; ++i)
    {
        free(files[i]);
    }
    free(files);

    return retval;
}
