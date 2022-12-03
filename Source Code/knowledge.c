/*
 * INF1002 (C Language) Group Project.
 *
 * This file implements the chatbot's knowledge base.
 *
 * knowledge_get() retrieves the response to a question.
 * knowledge_put() inserts a new response to a question.
 * knowledge_read() reads the knowledge base from a file.
 * knowledge_reset() erases all of the knowledge.
 * knowledge_write() saves the knowledge base in a file.
 *
 * You may add helper functions as necessary.
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "chat1002.h"

// Define a node strucutre that has an entity, intent, response and a pointer to the next node
typedef struct node
{
	char intent[MAX_INTENT];
	char entity[MAX_ENTITY];
	char response[MAX_RESPONSE];
	struct node *next;
} node;

// Declare a hashtable to store the question list headers
node *hashtable[MAX_HASHTABLE];

/*
 * Get the response to a question.
 *
 * Input:
 *   intent   - the question word
 *   entity   - the entity
 *   response - a buffer to receive the response
 *   n        - the maximum number of characters to write to the response buffer
 *
 * Returns:
 *   KB_OK, if a response was found for the intent and entity (the response is copied to the response buffer)
 *   KB_NOTFOUND, if no response could be found
 *   KB_INVALID, if 'intent' is not a recognised question word
 */
int knowledge_get(const char *intent, const char *entity, char *response, int n)
{
	// Hash the intent
	int index = hash(intent);

	// If the intent is valid, iterate through the question list
	if (index != -1)
	{
		// Create a cursor to iterate through the linked list
		node *cursor = hashtable[index];

		// If question list at index is empty, return question not found
		if (cursor == NULL)
		{
			return KB_NOTFOUND;
		}

		// Iterate through the linked list at index
		while (cursor != NULL)
		{
			// Check if the entity and intent matches
			if (compare_token(cursor->entity, entity) == 0 && compare_token(cursor->intent, intent) == 0)
			{
				// Check if response length is less than MAX_RESPONSE length
				if (strlen(cursor->response) < n)
				{
					// Copy the response into the response buffer and return KB_OK since question is found
					strcpy(response, cursor->response);
					return KB_OK;
				}
			}

			// Move the cursor to the next node
			cursor = cursor->next;
		}

		// Return KB_NOTFOUND if question is not found after iterating through all available nodes in the question list
		return KB_NOTFOUND;
	}

	// Return KB_INVALID if intent is invalid
	return KB_INVALID;
}

/*
 * Insert a new response to a question. If a response already exists for the
 * given intent and entity, it will be overwritten. Otherwise, it will be added
 * to the knowledge base.
 *
 * Input:
 *   intent    - the question word
 *   entity    - the entity
 *   response  - the response for this question and entity
 *
 * Returns:
 *   KB_FOUND, if successful
 *   KB_NOMEM, if there was a memory allocation failure
 *   KB_INVALID, if the intent is not a valid question word
 */
int knowledge_put(const char *intent, const char *entity, const char *response)
{
	// Hash the intent
	int index = hash(intent);

	// If the index is valid, put the knowledge into the question list
	if (index != -1)
	{
		// If question list at index is not empty, iterate through the linked list
		if (hashtable[index] != NULL)
		{
			// Create a cursor to iterate through the linked list
			node *cursor = hashtable[index];

			// Iterate through the linked list at index
			while (cursor != NULL)
			{
				// Check if the entity and intent matches
				if (compare_token(cursor->entity, entity) == 0 && compare_token(cursor->intent, intent) == 0)
				{
					// Replace current response for question with new response and return KB_OK since question is found
					strcpy(cursor->response, response);
					return KB_OK;
				}

				// Move the cursor to the next node
				cursor = cursor->next;
			}
		}

		// Create a new node
		node *new_node = malloc(sizeof(node));

		// Return KB_NOMEM if there is insufficient memory for allocation
		if (new_node == NULL)
		{
			return KB_NOMEM;
		}

		// Copy the entity, intent, and response into the new node
		strcpy(new_node->intent, intent);
		strcpy(new_node->entity, entity);
		strcpy(new_node->response, response);

		// Initialize the next pointer in new node to NULL
		new_node->next = NULL;

		// If question list is empty, add the new node to the hashtable
		if (hashtable[index] == NULL)
		{
			// Add new node to be the header of the question list and return KB_OK since new node was successful added
			hashtable[index] = new_node;
			return KB_OK;
		}
		// If linked list already exist, add new node to start of the list
		else
		{
			// Point new node to current header node and replace current header node in hashtable with new node, and return KB_OK since new node was successful added
			new_node->next = hashtable[index];
			hashtable[index] = new_node;
			return KB_OK;
		}
	}

	// Return KB_INVALID if intent is invalid
	return KB_INVALID;
}

/*
 * Read a knowledge base from a file.
 *
 * Input:
 *   f - the file
 *
 * Returns: the number of entity/response pairs successful read from the file
 */
int knowledge_read(FILE *f)
{
	const int buff_size = MAX_ENTITY + 1 + MAX_RESPONSE + 1;
	char buffer[buff_size]; // Buffer to hold read data from file
	const char *delimiters = "=[]\n"; 
	char intent[MAX_INTENT], entity[MAX_ENTITY], response[MAX_RESPONSE]; // Temp storage for intent, entity and response
	intent[0] = '\0';
	entity[0] ='\0';
	response[0] = '\0';
	int success_read = 0; // Counter for number of successful entity and response read into memory

	// Read line in file until it hits end of file
	while (!feof(f))
	{
		// Read line from file
		fgets(buffer, buff_size, f);

		// If line starts with '[' and ends with ']', means found section header
		if (buffer[0] == '[' && buffer[strlen(buffer - 2) == ']'])
		{
			// Remove special characters from section header and assign to intent
			strcpy(intent, strtok(buffer, delimiters));
			intent[strlen(intent)] = '\0';

			// Hash the intent
			int index = hash(intent);

			// If intent is valid, read subsequent lines in file for all the entities and responses for that intent
			if (index != -1)
			{
				// Create validator to check if read line is a valid entity and response
				int valid_line;

				// Read line for entity and response until it hits a line that does not contain an entity and response
				do
				{
					// Reset validator to default False
					valid_line = 0;

					// Read line from file
					fgets(buffer, buff_size, f);

					// If line is a valid entity and response pair, read into memory
					if (strchr(buffer, '=') != NULL)
					{
						// Set validator to True
						valid_line = 1;

						// Remove special characters and trailing newline and assign to entity and response
						strcpy(entity, strtok(buffer, delimiters));
						strcpy(response, strtok(NULL, delimiters));
						entity[strlen(entity)] = '\0';
						response[strlen(response)] = '\0';

						// Put the intent, entity and response into memory, and get the result of the operation
						int result = knowledge_put(intent, entity, response);

						// If knowledge_put operation was successful, add 1 to number of successful read ins
						if (result == KB_OK)
						{
							success_read++;
						}
						// If knowledge_put operation indicate a lack of memory, stop writing to memory and return KB_NOMEM
						else if (result == KB_NOMEM)
						{
							return KB_NOMEM;
						}
					}

				} while (valid_line);
			}
		}
	}

	// Return the number of successful read into memory
	return success_read;
}

/*
 * Reset the knowledge base, removing all know entitities from all intents.
 */
void knowledge_reset()
{
	// Iterate through through the hashtable
	for (int i = 0; i < MAX_HASHTABLE; i++)
	{
		// If question list at index is not empty, iterate through the linked list
		if (hashtable[i] != NULL)
		{
			// Create a cursor to iterate through the linked list
			node *cursor = hashtable[i];

			// Iterate through the linked list at index
			while (cursor != NULL)
			{
				// Free the memory of the current node and move the cursor to the next node
				node *next_node = cursor->next;
				free(cursor);
				cursor = next_node;
			}
		}

		// Reset index in hashtable to NULL
		hashtable[i] = NULL;
	}
}

/*
 * Write the knowledge base to a file.
 *
 * Input:
 *   f - the file
 */
void knowledge_write(FILE *f)
{
	char entity[MAX_ENTITY + 1], response[MAX_RESPONSE + 1]; // Temp storage for entity and response, +1 to compensate for '=' and '\n'

	// Iterate through through the hashtable
	for (int i = 0; i < MAX_HASHTABLE; i++)
	{
		// If question list at index is not empty, iterate through the linked list
		if (hashtable[i] != NULL)
		{
			// Create a cursor to iterate through the linked list
			node *cursor = hashtable[i];

			// If index is 0, write question list of "what" to file
			if (i == 0)
			{
				// Write section header "what" to file
				fputs("[what]\n", f);

				// Iterate through the linked list at index
				while (cursor != NULL)
				{
					// Copy entity and response from current node
					strcpy(entity, cursor->entity);
					strcpy(response, cursor->response);

					// Write entity and response to file
					strcat(entity, "=");
					fputs(entity, f);
					strcat(response, "\n");
					fputs(response, f);

					// Move the cursor to the next node
					cursor = cursor->next;
				}

				// Add a space between each section in the file
				fputc('\n', f);
			}
			// If index is 1, write question list of "where" to file
			else if (i == 1)
			{
				// Write section header "where" to file
				fputs("[where]\n", f);

				// Iterate through the linked list at index
				while (cursor != NULL)
				{
					// Copy entity and response from current node
					strcpy(entity, cursor->entity);
					strcpy(response, cursor->response);

					// Write entity and response to file
					strcat(entity, "=");
					fputs(entity, f);
					strcat(response, "\n");
					fputs(response, f);
					
					// Move the cursor to the next node
					cursor = cursor->next;
				}

				// Add a space between each section in the file
				fputc('\n', f);
			}
			// If index is 2, write question list of "who" to file
			else if (i == 2)
			{
				// Write section header "who" to file
				fputs("[who]\n", f);

				// Iterate through the linked list at index
				while (cursor != NULL)
				{
					// Copy entity and response from current node
					strcpy(entity, cursor->entity);
					strcpy(response, cursor->response);

					// Write entity and response to file
					strcat(entity, "=");
					fputs(entity, f);
					strcat(response, "\n");
					fputs(response, f);

					// Move the cursor to the next node
					cursor = cursor->next;
				}

				// Add a space between each section in the file
				fputc('\n', f);
			}
		}
	}
}

// Function to hash the intent into index in the hashtable
int hash(const char *str)
{
	// Return index 0 if intent matches "what"
	if (compare_token(str, "what") == 0)
	{
		return 0;
	}
	// Return index 1 if intent matches "where"
	else if (compare_token(str, "where") == 0)
	{
		return 1;
	}
	// Return index 2 if intent matches "who"
	else if (compare_token(str, "who") == 0)
	{
		return 2;
	}
	// Return -1 if the intent is invalid
	else
	{
		return -1;
	}
}