/*
 * INF1002 (C Language) Group Project.
 *
 * This file implements the behaviour of the chatbot. The main entry point to
 * this module is the chatbot_main() function, which identifies the intent
 * using the chatbot_is_*() functions then invokes the matching chatbot_do_*()
 * function to carry out the intent.
 *
 * chatbot_main() and chatbot_do_*() have the same method signature, which
 * works as described here.
 *
 * Input parameters:
 *   inc      - the number of words in the question
 *   inv      - an array of pointers to each word in the question
 *   response - a buffer to receive the response
 *   n        - the size of the response buffer
 *
 * The first word indicates the intent. If the intent is not recognised, the
 * chatbot should respond with "I do not understand [intent]." or similar, and
 * ignore the rest of the input.
 *
 * If the second word may be a part of speech that makes sense for the intent.
 *    - for WHAT, WHERE and WHO, it may be "is" or "are".
 *    - for SAVE, it may be "as" or "to".
 *    - for LOAD, it may be "from".
 * The word is otherwise ignored and may be omitted.
 *
 * The remainder of the input (including the second word, if it is not one of the
 * above) is the entity.
 *
 * The chatbot's answer should be stored in the output buffer, and be no longer
 * than n characters long (you can use snprintf() to do this). The contents of
 * this buffer will be printed by the main loop.
 *
 * The behaviour of the other functions is described individually in a comment
 * immediately before the function declaration.
 *
 * You can rename the chatbot and the user by changing chatbot_botname() and
 * chatbot_username(), respectively. The main loop will print the strings
 * returned by these functions at the start of each line.
 */

#include <stdio.h>
#include <string.h>
#include "chat1002.h"

/*
 * Get the name of the chatbot.
 *
 * Returns: the name of the chatbot as a null-terminated string
 */
const char *chatbot_botname()
{

	return "Chatbot";
}

/*
 * Get the name of the user.
 *
 * Returns: the name of the user as a null-terminated string
 */
const char *chatbot_username()
{

	return "User";
}

/*
 * Get a response to user input.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0, if the chatbot should continue chatting
 *   1, if the chatbot should stop (i.e. it detected the EXIT intent)
 */
int chatbot_main(int inc, char *inv[], char *response, int n)
{

	/* check for empty input */
	if (inc < 1)
	{
		snprintf(response, n, "");
		return 0;
	}

	/* look for an intent and invoke the corresponding do_* function */
	if (chatbot_is_exit(inv[0]))
		return chatbot_do_exit(inc, inv, response, n);
	else if (chatbot_is_load(inv[0]))
		return chatbot_do_load(inc, inv, response, n);
	else if (chatbot_is_question(inv[0]))
		return chatbot_do_question(inc, inv, response, n);
	else if (chatbot_is_reset(inv[0]))
		return chatbot_do_reset(inc, inv, response, n);
	else if (chatbot_is_save(inv[0]))
		return chatbot_do_save(inc, inv, response, n);
	else
	{
		snprintf(response, n, "I don't understand \"%s\".", inv[0]);
		return 0;
	}
}

/*
 * Determine whether an intent is EXIT.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "exit" or "quit"
 *  0, otherwise
 */
int chatbot_is_exit(const char *intent)
{
	return compare_token(intent, "exit") == 0 || compare_token(intent, "quit") == 0;
}

/*
 * Perform the EXIT intent.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after a question)
 */
int chatbot_do_exit(int inc, char *inv[], char *response, int n)
{
	snprintf(response, n, "Goodbye!");
	return 1;
}

/*
 * Determine whether an intent is LOAD.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "load"
 *  0, otherwise
 */
int chatbot_is_load(const char *intent)
{
	return compare_token(intent, "load") == 0;
}

/*
 * Load a chatbot's knowledge base from a file.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after loading knowledge)
 */
int chatbot_do_load(int inc, char *inv[], char *response, int n)
{
	FILE *f;					// File pointer created to locate the file
	int data_loaded = 0;		// Counter for number of successful entity and response data loaded into memory
	char file_name[MAX_ENTITY]; // Temp storage for file name
	file_name[0] = '\0';

	// If the user only typed in "load" but did not specify filename, prompt the user to include file name
	if (inc == 1 || inc < 2)
	{
		snprintf(response, n, "There is no file for me to read. Please specify file to load. e.g. 'sample.ini'");
		return 0;
	}
	// If index 2 of user input contains "from", e.g. load from hello.ini, ignore "from"
	else if (compare_token(inv[1], "from") == 0)
	{
		// Iterate through the words after index 2 ("from") and append the words to file_name
		for (int i = 2; i < inc; i++)
		{
			strcat(file_name, inv[i]);
			strcat(file_name, " ");
		}
	}
	// If index 2 of user input does not contain "from", e.g. load hello.ini
	else
	{
		// Iterate through the words after index 1 (intent) and append the words to file_name
		for (int i = 1; i < inc; i++)
		{
			strcat(file_name, inv[i]);
			strcat(file_name, " ");
		}
	}

	// Replace extra space with end of string indicator
	file_name[strlen(file_name) - 1] = '\0';

	// If specified file is not of type .ini, prompt the user to specify file of type .ini
	if (!compare_str_end_with(file_name, ".ini") && inc >= 2)
	{
		snprintf(response, n, "I cannot read the file. Please upload a .ini file. e.g. 'sample.ini'");
	}
	// If specified file is of type .ini, open file
	else if (compare_str_end_with(file_name, ".ini"))
	{
		// Open file in read mode
		f = fopen(file_name, "r");

		// If file does not open, inform user that file is not found
		if (f == NULL)
		{
			snprintf(response, n, "I cannot find the file. Please upload an existing .ini file.");
		}
		// If file is open correctly, read knowledge from file into memory
		else
		{
			// Read knowledge from file into memory and get the number of successful data loaded into memory
			data_loaded = knowledge_read(f);

			// Close file after reading
			fclose(f);

			// If there is insufficient memory, inform the user that there is insufficient memory space
			if (data_loaded == KB_NOMEM)
			{
				snprintf(response, n, "There is insufficient memory space. Please clear the knowledge in memory.");
			}
			// If knowledge_read operation was successful, inform user of number of successful data loaded into memory
			else
			{
				snprintf(response, n, "I have read %d responses from %s", data_loaded, file_name);
			}
		}
	}

	return 0;
}

/*
 * Determine whether an intent is a question.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "what", "where", or "who"
 *  0, otherwise
 */
int chatbot_is_question(const char *intent)
{
	return compare_token(intent, "what") == 0 || compare_token(intent, "where") == 0 || compare_token(intent, "who") == 0;
}

/*
 * Answer a question.
 *
 * inv[0] contains the the question word.
 * inv[1] may contain "is" or "are"; if so, it is skipped.
 * The remainder of the words form the entity.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after a question)
 */
int chatbot_do_question(int inc, char *inv[], char *response, int n)
{
	char intent[MAX_INTENT], entity[MAX_ENTITY], article[4]; // Temp storage for intent, article ("is" or "are") and entity
	intent[0] = '\0';
	entity[0] = '\0';
	article[0] = '\0';
	int get_result, put_result;

	// If the user input a single word and it is "what", "where" or "who", prompt user to enter a full question
	if (inc == 1)
	{
		if (compare_token(inv[0], "what") == 0)
		{
			snprintf(response, n, "I do not understand the phrase. Please enter a question. e.g. 'What is SIT?'");
		}
		else if (compare_token(inv[0], "where") == 0)
		{
			snprintf(response, n, "I do not understand the phrase. Please enter a question. e.g. 'Where is SIT?'");
		}
		else if (compare_token(inv[0], "who") == 0)
		{
			snprintf(response, n, "I do not understand the phrase. Please enter a question. e.g. 'Who is Frank Guan?'");
		}

		return 0;
	}
	// If the user input 2 words without an entity, check for article
	else if (inc == 2)
	{
		// If user input includes an article, e.g. "What is", prompt user to include a noun in the question
		if ((compare_token(inv[1], "is") == 0) || (compare_token(inv[1], "are") == 0))
		{
			snprintf(response, n, "Missing Noun. Please re-enter the question.");
			return 0;
		}
		// If user input does not include an article, e.g. "What SIT", save intent and entity from user input
		else
		{
			// Store intent and entity from user input
			strcpy(intent, inv[0]);
			strcpy(entity, inv[1]);
			intent[strlen(intent)] = '\0';
			entity[strlen(entity)] = '\0';
		}
	}
	// If the user input a 3 or more words, check if 2nd word of user input is a "is" or "are"
	else if (inc >= 3)
	{
		// If 2nd word of user input is a "is" or "are", store intent, article and entity
		if ((compare_token(inv[1], "is") == 0 || compare_token(inv[1], "are") == 0))
		{
			// Store the intent and article from user input
			strcpy(intent, inv[0]);
			strcpy(article, inv[1]);
			article[strlen(article)] = '\0';

			// Iterate through the words after index 2 (article) and append the words to entity
			for (int i = 2; i < inc; i++)
			{
				strcat(entity, inv[i]);
				strcat(entity, " ");
			}
		}
		// If 2nd word of user input is not a "is" or "are", store intent and entity
		else
		{
			// Store the intent from user input
			strcpy(intent, inv[0]);

			// Iterate through the words after index 1 (intent) and append the words to entity
			for (int i = 1; i < inc; i++)
			{
				strcat(entity, inv[i]);
				strcat(entity, " ");
			}
		}

		intent[strlen(intent)] = '\0';
		entity[strlen(entity) - 1] = '\0';
	}

	// Get knowledge from memory and get the outcome of the operation
	get_result = knowledge_get(intent, entity, response, n);

	// If knowledge_get operation was successful, return 0
	if (get_result == KB_OK)
	{
		return 0;
	}
	// If knowledge_read operation was successful due to knowledge not found, prompt user for new response to question
	else if (get_result == KB_NOTFOUND)
	{
		// If article is not empty, prompt user for new response with intent, article and entity
		if (article[0] != '\0')
		{
			prompt_user(response, MAX_RESPONSE, "I don't know. %s %s %s?", intent, article, entity);
		}
		// If article is empty, prompt user for new response with intent and entity
		else
		{
			prompt_user(response, MAX_RESPONSE, "I don't know. %s %s?", intent, entity);
		}

		// Put knowledge with new response from user into memory and get the outcome of the operation
		put_result = knowledge_put(intent, entity, response);

		// If knowledge_put operation was successful, thank the user
		if (put_result == KB_OK)
		{
			snprintf(response, n, "Thank you for the response.");
		}
		// If knowledge_put operation was unsuccessful due to invalid intent, inform the user of error
		else if (put_result == KB_INVALID)
		{
			snprintf(response, n, "Unknown Question, please re-type.");
		}
		// If knowledge_put operation was unsuccessful due to lack of memory, inform the user of error
		else if (put_result == KB_NOMEM)
		{
			snprintf(response, n, "Insufficient memory space. Please clear the knowledge in memory.");
		}
	}
	// If knowledge_get operation was unsuccessful due to invalid intent, inform the user of error
	else if (get_result == KB_INVALID)
	{
		snprintf(response, n, "Invalid Intent.");
	}

	return 0;
}

/*
 * Determine whether an intent is RESET.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "reset"
 *  0, otherwise
 *
 */
int chatbot_is_reset(const char *intent)
{
	return compare_token(intent, "reset") == 0;
}

/*
 * Reset the chatbot.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after beign reset)
 *
 */
int chatbot_do_reset(int inc, char *inv[], char *response, int n)
{
	// Call knowledge_reset to clear the hashtable of all data
	knowledge_reset();

	// Inform the user that the chatbot has been reset
	snprintf(response, n, "Chatbot reset.");
	return 0;
}

/*
 * Determine whether an intent is SAVE.
 *
 * Input:
 *  intent - the intent
 *
 * Returns:
 *  1, if the intent is "what", "where", or "who"
 *  0, otherwise
 *
 */
int chatbot_is_save(const char *intent)
{
	return compare_token(intent, "save") == 0;
}

/*
 * Save the chatbot's knowledge to a file.
 *
 * See the comment at the top of the file for a description of how this
 * function is used.
 *
 * Returns:
 *   0 (the chatbot always continues chatting after saving knowledge)
 *
 */
int chatbot_do_save(int inc, char *inv[], char *response, int n)
{
	FILE *f;					// File pointer created to locate the file
	char file_name[MAX_ENTITY]; // Temp storage for file name
	file_name[0] = '\0';

	// If the user only typed in "save" but did not specify filename, prompt the user to include file name
	if (inc == 1 || inc < 2)
	{
		snprintf(response, n, "There is no file for me to write to. Please specify file to save to. e.g. 'sample.ini'");
	}
	else
	{
		// Iterate through the user input
		for (int i = 0; i < inc; i++)
		{
			// If the word in the user input contains .ini, save word to file_name
			if (compare_str_end_with(inv[i], ".ini"))
			{
				strcpy(file_name, inv[i]);
				file_name[strlen(file_name)] = '\0';
			}
		}

		// If specified file is not of type .ini, prompt the user to specify file of type .ini
		if (file_name[0] == '\0')
		{
			snprintf(response, n, "I cannot read the file. Please upload a .ini file. e.g. 'sample.ini'");
		}
		// If specified file is of type .ini, open and write to file
		else
		{
			// Open/create file in write mode with user specified file name
			f = fopen(file_name, "w");

			// If file is open correctly, write knowledge from memory into file
			if (f != NULL)
			{
				// Write knowledge from memory into file
				knowledge_write(f);

				// Inform user that knowledge_write operation was successful
				snprintf(response, n, "My knowledge has been saved to %s", file_name);
			}
			// If file does not open, inform user that file is unable to be opened/created
			else
			{
				snprintf(response, n, "I am unable to open/create file. Please try again.");
			}

			// Close the file after writing
			fclose(f);
		}
	}

	return 0;
}

// Utility function to check if string ends with substring
int compare_str_end_with(const char *str, const char *substr)
{
	int str_len = strlen(str), substr_len = strlen(substr);

	// If string length is more than substring length, check if string ends with substring
	if (str_len >= substr_len)
	{
		// Iterate through the end of the string and compare against the substring
		for (int i = 0; i < substr_len; i++)
		{
			// If any character at the end of the string does not match the substring character at the same relative index position, return 0
			if (str[str_len - substr_len + i] != substr[i])
			{
				return 0;
			}
		}

		// If the end of the string matched the substring, return 1
		return 1;
	}

	// If string length is less than substring length, return 0
	return 0;
}