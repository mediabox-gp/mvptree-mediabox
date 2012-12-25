/*

    MVPTree c library
    Copyright (C) 2008-2009 Aetilius, Inc.
    All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    D Grant Starkweather - dstarkweather@phash.org

*/

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

#define MVP_BRANCHFACTOR 2
#define MVP_PATHLENGTH   5
#define MVP_LEAFCAP      25

static unsigned long long nbcalcs = 0;


float hamming_distance_cb(MVPDP *pointA, MVPDP *pointB)
{
    uint64_t a = *((uint64_t*)pointA->data);
    uint64_t b = *((uint64_t*)pointB->data);

    int res = ph_hamming_distance(a, b);

    return (float)res;
}

int main(int argc, char **argv){
    if (argc < 4){
	printf("not enough input args\n");
	printf(" %s  command filename [directory] [radius]\n\n", argv[0]);
	printf("  command    - command - e.g. 'add', 'query' or 'print'\n");
	printf("  filename   - file from which to read the tree\n");
	printf("  directory  - directory (for add and query)\n");
	printf("  radius     - radius for query operation (default = 21.0)\n");
	printf("  knearset   - knearest for query operation (default = 5)\n");
	return 0;
    }

    const char *command  = argv[1];
    const char *filename = argv[2];
    const char *dirname  = argv[3];
    float radius = 21;
    int knearest = 5;
    if (argc > 4)
        radius = atof(argv[4]);
    if (argc > 5)
        knearest = atoi(argv[5]);

    printf("command  - %s\n", command);
    printf("filename - %s\n", filename);
    printf("dir      - %s\n", dirname);
    printf("radius   - %f\n", radius);
    printf("knearest - %d\n", knearest);

    CmpFunc distance_func = hamming_distance_cb;

    int nbfiles;
    unsigned int matches = 0;
    char **files = ph_readfilenames(dirname, nbfiles);
    assert(files);

    fprintf(stdout,"\n %d files in %s\n\n", nbfiles, dirname);

    MVPDP **points = (MVPDP**)malloc(nbfiles*sizeof(MVPDP*));
    assert(points);

    MVPError err;
    MVPTree *tree = mvptree_read(filename,distance_func,MVP_BRANCHFACTOR, MVP_PATHLENGTH,\
                                                                         MVP_LEAFCAP, &err);
    assert(tree);

    if (!strncasecmp(command,"add",3) || !strncasecmp(command,"query",3)){
	int count = 0;
	ulong64 hashvalue;
	for (int i=0;i < nbfiles;i++){
	    char *name = strrchr(files[i],'/')+1;

	    if (ph_dct_imagehash(files[i], hashvalue) < 0){
		printf("Unable to get hash value.\n");
		continue;
	    }
	    printf("(%d) %llx %s\n", i, (unsigned long long)hashvalue, files[i]);

	    points[count] = dp_alloc(MVP_UINT64ARRAY);
	    points[count]->id = strdup(name);
	    points[count]->data = malloc(1*MVP_UINT64ARRAY);
	    points[count]->datalen = 1;
	    memcpy(points[count]->data, &hashvalue, MVP_UINT64ARRAY);
	    count++;
	}

	printf("\n");

	if (!strncasecmp(command,"add", 3)){

	    printf("Add %d hashes to tree.\n", count);
	    MVPError error = mvptree_add(tree, points, count);
	    if (error != MVP_SUCCESS){
		printf("Unable to add hash values to tree.\n");
		goto cleanup;
	    }

	    printf("Save file.\n");
	    error = mvptree_write(tree, filename, 00755);
	    if (error != MVP_SUCCESS){
		printf("Unable to save file.\n");
		goto cleanup;
	    }

	} else if (!strncasecmp(command,"query", 3)){

	    unsigned int nbresults;
	    for (int i=0;i<count;i++){
		printf("\n(%d) looking up %s ...\n", i, files[i]);
		nbcalcs = 0;
		MVPDP **results = mvptree_retrieve(tree,points[i],knearest,\
                                                               radius, &nbresults, &err);
		if (nbresults > 0)
		{
			matches++;
			printf("-----------%d results (%d calcs)--------------\n",nbresults,nbcalcs);
			for (int j=0;j<nbresults;j++){
		  	  	printf("    (%d) %s\n", j, results[j]->id);
			}
			printf("-----------------------------------------------\n");
			//printf("Hit enter key.\n");
			//getchar();
		}
		free(results);
	    }
	}
    } else if (!strncasecmp(command,"print", 3)){
	printf("-----------------------print-------------------------\n");
	mvptree_print(stdout,tree);
	printf("-----------------------------------------------------\n\n");
    }

    if (matches > 0)
    {
	    printf("Matches: %u\n", matches);
    }
    mvptree_clear(tree, free);

cleanup:
    for (int i=0;i<nbfiles;i++){
	free(files[i]);
    }
    free(files);


    return 0;
}
