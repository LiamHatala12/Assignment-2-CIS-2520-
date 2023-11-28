#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "LLvNode.h"  // Include your custom linked list header
#include "fasta.h"   // Include your FASTA handling header

// Function to process FASTA records from a file and measure time taken
int processFasta(char *filename, double *timeTaken)
{
    FILE *fp;
    FASTArecord fRecord;
    int lineNumber = 0, recordNumber = 0, status;
    int eofSeen = 0;
    LLvNode *CNode = NULL;  // Current node
    LLvNode *PNode = NULL;  // Previous node
    LLvNode *LNode = NULL;  // Last node
    clock_t startTime, endTime;

    // Open the file for reading
    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failure opening %s : %s\n", filename, strerror(errno));
        return -1;
    }

    // Record the time before processing
    startTime = clock();

    do {
        // Print a '.' every 10,000 records to show progress
        if ((recordNumber % 10000) == 0) {
            printf(".");
            fflush(stdout);
        }

        // Initialize the FASTA record
        fastaInitializeRecord(&fRecord);

        // Read a FASTA record from the file
        status = fastaReadRecord(fp, &fRecord);
        if (status == 0) {
            eofSeen = 1;
        } else if (status > 0) {
			
            // Create a new linked list node with the FASTA record and append it
            CNode = llNewNode(fRecord.description, &fRecord);

            // If the list is empty, both the head and tail point to CNode
            if (PNode == NULL) {

                // Set LNode to CNode
                LNode = CNode;

                // Set PNode to CNode
                PNode = CNode;

            } else {

                // LNode looks to the next node and makes it the current node
                LNode->next = CNode;

                // Set LNode to CNode
                LNode = CNode;

                // Set the next pointer of LNode to NULL
                LNode->next = NULL;

            }

            lineNumber += status;
            recordNumber++;

            // Clear the FASTA record for reuse
            fastaClearRecord(&fRecord);
        } else {
            fprintf(stderr, "status = %d\n", status);
            fprintf(stderr, "Error: failure at line %d of '%s'\n", lineNumber, filename);
            return -1;
        }
    } while (!eofSeen);

    printf(" %d FASTA records\n", recordNumber);

    // Record the time after processing
    endTime = clock();

    (*timeTaken) = ((double)(endTime - startTime)) / CLOCKS_PER_SEC;

    // Close the file
    fclose(fp);

    // Free the linked list using the function in LLvNode
    llFree(PNode, NULL, NULL);

    return recordNumber;
}

// Function to process FASTA records repeatedly
int processFastaRepeatedly(char *filename, long repeatsRequested)
{
    double timeThisIterationInSeconds;
    double totalTimeInSeconds = 0;
    int minutesPortion;
    int status;
    long i;

    for (i = 0; i < repeatsRequested; i++) {
        status = processFasta(filename, &timeThisIterationInSeconds);
        if (status < 0)
            return -1;
        totalTimeInSeconds += timeThisIterationInSeconds;
    }

    printf("%lf seconds taken for processing total\n", totalTimeInSeconds);

    totalTimeInSeconds /= (double) repeatsRequested;

    minutesPortion = (int) (totalTimeInSeconds / 60);
    totalTimeInSeconds = totalTimeInSeconds - (60 * minutesPortion);
    printf("On average: %d minutes, %lf second per run\n", minutesPortion, totalTimeInSeconds);

    return status;
}

// Function to display program usage information
void usage(char *progname)
{
    fprintf(stderr, "%s [<OPTIONS>] <file> [ <file> ...]\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "Prints timing of loading and storing FASTA records.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options: \n");
    fprintf(stderr, "-R <REPEATS> : Number of times to repeat load.\n");
    fprintf(stderr, "             : Time reported will be average time.\n");
    fprintf(stderr, "\n");
}

/**
 * Program mainline
 */
int main(int argc, char **argv)
{
    int i, recordsProcessed = 0;
    long repeatsRequested = 1;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'R') {
                if (i >= argc) {
                    fprintf(stderr, "Error: need argument for repeats requested\n");
                    return 1;
                }
                if (sscanf(argv[++i], "%ld", &repeatsRequested) != 1) {
                    fprintf(stderr, "Error: cannot parse repeats requested from '%s'\n", argv[i]);
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: unknown option '%s'\n", argv[i]);
                usage(argv[0]);
            }
        } else {
            recordsProcessed = processFastaRepeatedly(argv[i], repeatsRequested);
            if (recordsProcessed < 0) {
                fprintf(stderr, "Error: Processing '%s' failed -- exiting\n", argv[i]);
                return 1;
            }
            printf("%d records processed from '%s'\n", recordsProcessed, argv[i]);
        }
    }

    if (recordsProcessed == 0) {
        fprintf(stderr, "No data processed -- provide the name of a file on the command line\n");
        usage(argv[0]);
        return 1;
    }

    return 1;
}
