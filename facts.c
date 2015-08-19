/* facts.c 
 *	:author Jetharin Kevin Charoenworawat
 *	:UTEID	JKC2237
 *
 *	:intent Program that creates a database
 * 		from a file containing various 
 *		facts.  The user can then search
 *		for different facts via another
 *		file that contains questions.
 *
 *
 *	:slips  1 Slip Day Use
 */


// Boolean Type Definition
typedef short Boolean;
#define TRUE 1
#define FALSE 0

struct Fact
{
	char* fact_subject;
	char* fact_name;
	char* fact_values;
	
	char* completeFactLine;
};

struct Node
{
	struct Node* previous;
	struct Fact* fact;
	struct Node* next;

	Boolean found;
};

struct Linked_List
{
	struct Node* first;
	struct Node* last;

	int numStats;
};

#include <stdio.h>
#include <stdlib.h> // for malloc()
#include <string.h> // for strcpy()

FILE* facts_file;
FILE* questions_file;
struct Linked_List Facts = {NULL, NULL, 0};
Boolean processingFacts = TRUE;
struct Linked_List Questions = {NULL, NULL, 0};
char** allLines;

void printLinkedLists();
void link(struct Node* temp); 
void linkQ(struct Node* temp);
void getFacts();
void unlink(struct Node* temp);

// Method that creates facts based on the lines of input.
//	Assumes that facts will be uniform in syntax.
void createList(int bufferSize, int numLines)
{
	char* temp;
	char** tempAllLines = allLines;
	int numLinesToProcess;

	int i;
	int offset;

	for(numLinesToProcess = numLines; numLinesToProcess > 0; numLinesToProcess--)
	{
		// Create a copy of a single line from allLines
		temp = (char*)(malloc(bufferSize * sizeof(char)));	
		if(temp == NULL)
		{
			fprintf(stderr, "ERROR: Out of Memory\n");
			exit(1);
		}
		strcpy(temp, * tempAllLines); // can use memcpy instead with bufferSize
		free(* tempAllLines);

		if(temp[0] != 'F' && temp[0] != 'Q' && temp[0] != 0)
		{
			fprintf(stderr, "ERROR: First letter is neither 'F' nor 'Q'. Skipping the line: %s\n", temp);	
		}
		else if(temp[0] != 0)
		{
			if(temp[1] == 32 && temp[2] != 32)
			{
				i = 2; // Adjust i as to skip the initial info type signal

				struct Fact* tempFact;
				tempFact = (struct Fact*)(malloc(sizeof(struct Fact)));
				if(tempFact == NULL)
				{
					fprintf(stderr, "ERROR: Out of Memory\n");
					exit(1);
				}

				(* tempFact).completeFactLine = temp;


				// Get fact_subject
				char* tempFactSubject;
				tempFactSubject = (char*)(malloc(bufferSize * sizeof(char)));
				if(tempFactSubject == NULL)
				{
					fprintf(stderr, "ERROR: Out of Memeory\n");
					exit(1);
				}
	
				offset = i;
				while(temp[i] != ':')
				{
					if(i == strlen(temp)-1)
					{
						fprintf(stderr, "ERROR: Bad Syntax - no ':' delimiter. Skipping the line: %s\n", temp);
						free(temp);
						free(tempFact);
						free(tempFactSubject);
						break;
					}

					tempFactSubject[i-offset] = temp[i];	
					
					i++;
				}
				
				(* tempFact).fact_subject = tempFactSubject;


				// i is currenty at the colon at this point in the code
				if(temp[i+1] != 32 || temp[i+2] == 32)
				{
					fprintf(stderr, "ERROR: Bad Syntax - incorrect format. Skipping the line: %s\n", temp);
					free(temp);
					free(tempFact);
					free(tempFactSubject);
					break;
				}							
	
				i = i+2; // Adjust i to get at the Fact's name


				// Get fact_name
				char* tempFactName;
				tempFactName = (char*)(malloc((bufferSize-i) * sizeof(char)));	
				if(tempFactName == NULL)
				{
					fprintf(stderr, "ERROR: Out of Memeory\n");
					exit(1);
				}
				
				offset = i;
				if(processingFacts)
				{	
					while(temp[i] != '=')
					{
						if(i == strlen(temp)-1)
						{
							fprintf(stderr, "ERROR: Bad Syntax - no '=' delimiter. Skipping the line: %s\n", temp);
							free(temp);
							free(tempFact);
							free(tempFactSubject);
							free(tempFactName);
							break;
						}
		
						tempFactName[i-offset] = temp[i];

						i++;
					}
				
					(* tempFact).fact_name = tempFactName;


					// i is currenty at the equals delimiter at this point in the code
					if(temp[i+1] == 32)
					{
						fprintf(stderr, "ERROR: Bad Syntax of Fact. Skipping the line: %s\n", temp);
						free(temp);
						free(tempFact);
						free(tempFactSubject);
						free(tempFactName);
						break;
					}
					
					i++; // Adjust i past the equals


					// Get fact_values	
					char* tempFactValues;
					tempFactValues = (char*)(malloc((bufferSize-i) * sizeof(char)));
					if(tempFactValues == NULL)
					{
						fprintf(stderr, "ERROR: Out of Memeory\n");
						exit(1);
					}
				
					offset = i;	
					while(i < strlen(temp))
					{
						tempFactValues[i-offset] = temp[i];
					
						i++;
					}
	
					(* tempFact).fact_values = tempFactValues;
				}
				else
				{
					// Get fact_name for Question
					while(i < strlen(temp))
					{
						tempFactName[i-offset] = temp[i];

						i++;
					}
					
					(* tempFact).fact_name = tempFactName;
				}


				// Create a Node from the Fact
				struct Node* tempNode;
				tempNode = (struct Node*)(malloc(sizeof(struct Node)));	
				if(tempNode == NULL)
				{
					fprintf(stderr, "ERROR: Out of Memory\n");
					exit(1);
				}
				
				(* tempNode).fact = tempFact;
				(* tempNode).next = NULL;


				// Add tempNode to Facts: the Linked List
				if(processingFacts)
				{
					link(tempNode);
				}
				else
				{
					linkQ(tempNode);
				}
			}
			else
			{
				fprintf(stderr, "ERROR: Bad Syntax of Fact. Skipping the line: %s\n", temp);
				free(temp);
			}
		}

		tempAllLines++; // Update Pointer to next line 
	}
}

void link(struct Node* nodeToLink) // Method that links a node to the end of the list
{
	if(Facts.first == NULL)
	{
		(* nodeToLink).previous = NULL;

		Facts.first = nodeToLink;
		Facts.last = nodeToLink;
	}
	else
	{
		Boolean noDuplicate = TRUE;
		// Search for duplicate fact
		struct Node* tempNode;
		tempNode = Facts.first;
		while(tempNode != NULL)
		{
			if( strcmp((* (* nodeToLink).fact).fact_subject, (* (* tempNode).fact).fact_subject) == 0
				&& strcmp((* (* nodeToLink).fact).fact_name, (* (* tempNode).fact).fact_name) == 0)
			{
				fprintf(stderr, "%s\n", (* (* tempNode).fact).fact_values);
				strcpy((* (* tempNode).fact).fact_values, (* (* nodeToLink).fact).fact_values);
				fprintf(stderr, "%s\n", (* (* tempNode).fact).fact_values);
				
				noDuplicate = FALSE;

				// Free the memory that was allocated to the new node/fact
				//	that is no longer needed.
				// Node's Fact parameters	
				free((* (* nodeToLink).fact).fact_subject);
				free((* (* nodeToLink).fact).fact_name);
				free((* (* nodeToLink).fact).fact_values);
				free((* (* nodeToLink).fact).completeFactLine);
				// Node pointers
				free((* nodeToLink).previous);
				free((* nodeToLink).next);
				free((* nodeToLink).fact);
				
				free(nodeToLink);

				break;
			}

			tempNode = (* tempNode).next;
		}
		
		if(noDuplicate)
		{
			(* nodeToLink).previous = Facts.last;
			(* (Facts.last)).next = nodeToLink;	

			Facts.last = nodeToLink;
		}	
	}

	Facts.numStats++;
}
void linkQ(struct Node* nodeToLink)
{
	if(Questions.first == NULL)
	{
		(* nodeToLink).previous = NULL;
		
		Questions.first = nodeToLink;
		Questions.last = nodeToLink;
	}
	else
	{
		(* nodeToLink).previous = Questions.last;
		(* (Questions.last)).next = nodeToLink;

		Questions.last = nodeToLink;
	}

	(* nodeToLink).found = FALSE;
	Questions.numStats++;
}

// Method that Creates a 2D Array of 'strings' from the fact file.
//	The 2D Array (allLines) is then used in a method call in 
//	order to create the list of facts. 
void Process_Facts()
{
	char c;
	
	int lineBufferSize = 100;
	int numLinesBufferSize = 20;
	char* buffer = (char*)(malloc(lineBufferSize * sizeof(char)));
	allLines = (char**)(malloc(numLinesBufferSize * sizeof(char*)));
		// Assuming there will be enough memory for initalization.

	int numLines = 0;	
	int i = 0;

	while((c = getc(facts_file)) != EOF)	
	{
		if(c == '\n')
		{

			numLines++;
			if(numLines == numLinesBufferSize - 1)
			{
				numLinesBufferSize = numLinesBufferSize * 2;

				allLines = (char**)(realloc(allLines, (numLinesBufferSize * sizeof(char*))));
				if(allLines == NULL)
				{
					fprintf(stderr, "ERROR: Out of Memory");
					exit(1);
				}
			}

			// ??? Instead of lineBufferSize
			(* (allLines + numLines-1)) = (char*)(malloc(lineBufferSize * sizeof(char))); 
			memcpy((* (allLines + (numLines-1))), buffer, lineBufferSize);

			// Reset Buffer in order to eliminate overlap of facts	
			memset(buffer, 0, lineBufferSize);

			// Reset Index for char placement
			i = 0;
		}
		else
		{
			// Realloc Memory if more space to store the line is needed
			if(i == lineBufferSize - 1)
			{
				lineBufferSize = lineBufferSize * 2;
				
				buffer = (char*)(realloc(buffer, (lineBufferSize * sizeof(char))));	
				if(buffer == NULL)
				{
					fprintf(stderr, "ERROR: Out of Memory");
					exit(1);
				}
			}

			buffer[i] = c;
			i++;
		}
	}	
	
	createList(lineBufferSize, numLines);
	processingFacts = FALSE;

	free(buffer);
	free(allLines);
}

// Method that stores the individual lines from questions_file 
//	to be stored in a Linked_List via createList()	
void Process_Questions()
{
	char c;
	
	int lineBufferSize = 100;
	int numLinesBufferSize = 20;
	char* buffer = (char*)(malloc(lineBufferSize * sizeof(char)));
	allLines = (char**)(malloc(numLinesBufferSize * sizeof(char*)));
		// Assuming there will be enough memory for initalization.

	int numLines = 0;	
	int i = 0;

	while((c = getc(questions_file)) != EOF)	
	{
		if(c == '\n')
		{

			numLines++;
			if(numLines == numLinesBufferSize - 1)
			{
				numLinesBufferSize = numLinesBufferSize * 2;

				allLines = (char**)(realloc(allLines, (numLinesBufferSize * sizeof(char*))));
				if(allLines == NULL)
				{
					fprintf(stderr, "ERROR: Out of Memory\n");
					exit(1);
				}
			}

			// ??? Instead of lineBufferSize
			(* (allLines + numLines-1)) = (char*)(malloc(lineBufferSize * sizeof(char))); 
			memcpy((* (allLines + (numLines-1))), buffer, lineBufferSize);

			// Reset Buffer in order to eliminate overlap of facts	
			memset(buffer, 0, lineBufferSize);

			// Reset Index for char placement
			i = 0;
		}
		else
		{
			// Realloc Memory if more space to store the line is needed
			if(i == lineBufferSize - 1)
			{
				lineBufferSize = lineBufferSize * 2;
				
				buffer = (char*)(realloc(buffer, (lineBufferSize * sizeof(char))));	
				if(buffer == NULL)
				{
					fprintf(stderr, "ERROR: Out of Memory\n");
					exit(1);
				}
			}

			buffer[i] = c;
			i++;
		}
	}	

	createList(lineBufferSize, numLines);
	getFacts();

	free(buffer);
	free(allLines);
}

// Method that will take the Questions Linked List 
//	and output the corresponding facts from Facts.
void getFacts()
{
	struct Node* qIndex = Questions.first;
	struct Node* fIndex = Facts.first;
	Boolean done = FALSE;
	Boolean foundOne = FALSE;

	while(qIndex != NULL && !done)
	{
		while(fIndex != NULL && !foundOne)
		{
			if( strcmp((* (* qIndex).fact).fact_subject, (* (* fIndex).fact).fact_subject) == 0
				&& strcmp((* (* qIndex).fact).fact_name, (* (* fIndex).fact).fact_name) == 0)
			{
				if((* qIndex).next == NULL)
				{
					done = TRUE;
				}
				

				fprintf(stdout, "%s\n", (* (* fIndex).fact).completeFactLine);


				(* qIndex).found = TRUE;
				foundOne = TRUE;
			}
			else
			{
				fIndex = (* fIndex).next;
			}
		}

		if(!done)
		{
			qIndex = (* qIndex).next;

			if(foundOne)
			{
				unlink((* qIndex).previous);
			}
		
			fIndex = Facts.first;
			foundOne = FALSE;
		}
	}

	// Output rest of the remaining questions with unkown values
	int i;

	qIndex = Questions.first;
	while(qIndex != NULL)
	{
		if((* qIndex).found == FALSE)
		{

			fprintf(stdout, "F ");
			for(i = 2; i < strlen((* (*qIndex).fact).completeFactLine); i++)
			{
				fprintf(stdout, "%c", (* (* qIndex).fact).completeFactLine[i]);
			}

			fprintf(stdout, "=unkown\n");
		}

		qIndex = (* qIndex).next;
	}

}

// Method that removes a desired node and frees its data.
//	This method is used exclusively for removing 
//	questions. As such, the fact_values parameter
//	does not to be freed.
void unlink(struct Node* nodeToUnlink)
{
	// Case: node is first in the linked list
	if((* nodeToUnlink).previous == NULL)
	{
		(* (* nodeToUnlink).next).previous = NULL;

		Questions.first = (* nodeToUnlink).next;
	}
	// Case: node is the last in the linked list
	else if((* nodeToUnlink).next == NULL)
	{
		(* (* nodeToUnlink).previous).next = NULL;

		Questions.last = (* nodeToUnlink).previous;
	}
	else
	{
		(* (* nodeToUnlink).previous).next = (* nodeToUnlink).next;	
		(* (* nodeToUnlink).next).previous = (* nodeToUnlink).previous;
	}

	// Free the node's Fact parameters	
	free((* (* nodeToUnlink).fact).fact_subject);
	free((* (* nodeToUnlink).fact).fact_name);
	free((* (* nodeToUnlink).fact).completeFactLine);
	// Free the Node pointers
	//free((* nodeToUnlink).previous);
	free((* nodeToUnlink).next);
	free((* nodeToUnlink).fact);
	
	free(nodeToUnlink);

	Questions.numStats--;
}	
 
// Program's Driver Method
int main(int argc, char** argv)
{
	// Skips the Program Name as an Argument
	argc--;
	argv++;

	// Case: too many arguments
	if(argc > 2)
	{
		fprintf(stderr, "ERROR: Invalid Number of Arguments Passed.");
		exit(1);
	}
	else if(argc == 0)
	{
		fprintf(stderr, "ERROR: No Facts File Given.");
		exit(0);
	}

	facts_file = fopen(*argv, "r");
	if(facts_file == NULL)
	{
		fprintf(stderr, "Can't open %s\n", *argv);
		exit(1);
	}
	argc--;
	argv++;
	
	if(argc == 1)
	{
		questions_file = fopen(*argv, "r");
		if(questions_file == NULL)
		{
			fprintf(stderr, "Can't open %s\n", *argv);
			exit(1);
		}
	}
	else
	{
		fprintf(stdout, "Enter the questions desired in the correct syntax.\n");
		questions_file = stdin;
	}

	Process_Facts();
	Process_Questions();

	fclose(facts_file);
	fclose(questions_file);
	exit(0);
}

// Method to see the contents of the two linked lists
void printLinkedLists()
{
	fprintf(stdout, "/////Facts Contents////\n");
	struct Node* in = Facts.first;
	while(in != NULL)
	{
//		fprintf(stdout, "Fact Subject: %s\n", (* (* in).fact).fact_subject);	
//		fprintf(stdout, "Fact Name: %s\n", (* (* in).fact).fact_name);	
//		fprintf(stdout, "Fact Values: %s\n", (* (* in).fact).fact_values);	
		fprintf(stdout, "%s\n", (* (* in).fact).completeFactLine);	
		
		in = (* in).next;
	}
		fprintf(stdout, "Number of Facts: %d\n\n", Facts.numStats);	

	fprintf(stdout, "/////Questions Contents/////\n");
	in = Questions.first;
	while(in != NULL)
	{
//		fprintf(stdout, "Fact Subject: %s\n", (* (* in).fact).fact_subject);	
//		fprintf(stdout, "Fact Name: %s\n", (* (* in).fact).fact_name);	
//		fprintf(stdout, "Fact Values: %s\n", (* (* in).fact).fact_values);	
		fprintf(stdout, "%s\n", (* (* in).fact).completeFactLine);		

		in = (* in).next;
	}
}
