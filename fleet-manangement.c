#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Constants
#define MAX_BOATS 120
#define MAX_NAME_LENGTH 128

// Monthly rate constants for different boat storage
#define SLIP_RATE 12.50
#define LAND_RATE 14.00
#define TRAILOR_RATE 25.00
#define STORAGE_RATE 11.20

// Enum for different boat location types
typedef enum {
	
	slip,
	land, 
	trailor,
	storage	

} LocationType;

// Union to hold type-specific info
typedef union {

	int slip_number;
	char bay_letter;
	char trailor_tag[16];
	int storage_number;	

} LocationInfo;

// Struct for holding boat info
typedef struct {

	char name[MAX_NAME_LENGTH];
	int length;
	LocationType type;
	LocationInfo info;
	double amount_owed;

} Boat;

// Parse line from CSV into Boat struct
Boat *createBoatCSV(char *line) {

	Boat *b = malloc(sizeof(Boat));
	if (!b) {	
		perror("ERROR");
		return NULL;
	}

	// Parse boat name
	char *token = strtok(line, ",");
	if (!token) {
		return NULL;
	}
	strncpy(b->name, token, MAX_NAME_LENGTH);

	// Parse boat length
	token = strtok(NULL, ",");
	if (!token) {
		return NULL;
	}
	b->length = atoi(token);

	//Parse boat type
	token = strtok(NULL, ",");
	if (!token) {
		return NULL;
	}
	if (strcmp(token, "slip") == 0) {
		b->type = slip;
	}
	else if (strcmp(token, "land") == 0) {
		b->type = land;
	}
	else if (strcmp(token, "trailor") == 0) {
		b->type = trailor;
	}
	else if (strcmp(token, "storage") == 0){
		b->type = storage;
	} else {
		return NULL;
	}

	// Parse type-specific
	token = strtok(NULL, ",");
	if (!token) {
		return NULL;
	}
	switch (b->type) {
		case slip:
			b->info.slip_number = atoi(token);
			break;
		case land:
			b->info.bay_letter = token[0];
			break;
		case trailor:
			strncpy(b->info.trailor_tag, token, sizeof(b->info.trailor_tag));
			break;
		case storage:
			b->info.storage_number = atoi(token);
			break;
	}

	// Parse amount owed
	token = strtok(NULL, ",");
	if (!token) {
		return NULL;
	}
	b->amount_owed = atof(token);

	return b;

}

// Load the boats from CSV into boats array
int loadBoats(const char *filename, Boat *boats[]) {

	FILE *fp = fopen(filename, "r");
	if (!fp) {
		perror("ERROR");
		return 0;
	}
	char line [256];
	int count = 0;

	// Reads each line and parses into Boat
	while (fgets(line, sizeof(line), fp) && count < MAX_BOATS) {
		line[strcspn(line, "\n")] = '\0';
		Boat *b = createBoatCSV(line);
		if (b) {
			boats[count++] = b;
		}
	}

	fclose(fp);
	return count;

}

// Compare boat names alphabetically 
int compareBoats(const void *a, const void *b) {

	Boat *ba = *(Boat **)a;
	Boat *bb = *(Boat **)b;
	return strcasecmp(ba->name, bb->name);

}

// Print sorted inventory of boats
void printInventory(Boat *boats[], int count) {

	// Sort boats before printing
	qsort(boats, count, sizeof(Boat *), compareBoats);

	for (int i = 0; i < count; i++) {
		Boat *b = boats[i];
		// Print boat name and length
		printf("%-20s %3d' ", b->name, b->length);

		// Print location info
		switch (b->type) {
			case slip:
				printf("   slip   # %2d", b->info.slip_number);
				break;
			case land:
				printf("   land      %c", b->info.bay_letter);
				break;
			case trailor:
				printf("trailor %s", b->info.trailor_tag);
				break;
			case storage:
				printf("storage   # %2d", b->info.storage_number);
				break;
		}

		// Print amount owed
		printf("   Owes $%7.2f\n", b->amount_owed);
	}

}

// Insert a new boat into the array (sorted)
int insertBoat(Boat *boats[], int count, Boat *new_boat) {

	if (count >= MAX_BOATS) {
		printf("Marina is full\n");
		return count;
	}

	// Find position to insert boat alphabetically
	int pos = 0;
	while (pos < count && strcasecmp(new_boat->name, boats[pos]->name) > 0) {
		pos++;
	}

	// Shifts boats to make room for new boat
	for (int i = count; i > pos; i--) {
		boats[i] = boats[i-1];
	}

	boats[pos] = new_boat;
	return count + 1;

}

// Find the index of a boat by name
int findBoat(Boat *boats[], int count, const char *name) {
	
	// Linear search for boat name
	for (int i = 0; i < count; i++) {
		if (strcasecmp(boats[i]->name, name) == 0) {
			return i;
		}
	}
	return -1;

}

// Save the current boat data to CSV
void saveBoats(const char *filename, Boat *boats[], int count) {
	
	FILE *fp = fopen(filename, "w");
	if (!fp) {
		perror("ERROR");
		return;
	}

	// Write boat in CSV
	for (int i = 0; i < count; i++) {
		Boat *b = boats[i];
		switch (b->type) {
			case slip:
				fprintf(fp, "%s,%d,slip,%d,%.2f\n", b->name, b->length, b->info.slip_number, b->amount_owed);
				break;
			case land:
				fprintf(fp, "%s,%d,land,%c,%.2f\n", b->name, b->length, b->info.bay_letter, b->amount_owed);
				break;
			case trailor:
				fprintf(fp, "%s,%d,trailor,%s,%.2f\n", b->name, b->length, b->info.trailor_tag, b->amount_owed);
				break;
			case storage:
				fprintf(fp, "%s,%d,storage,%d,%.2f\n", b->name, b->length, b->info.storage_number, b->amount_owed);
				break;
		}
	}

	fclose(fp);

}

// Free memory
void freeBoats(Boat *boats[], int count) {
	for (int i = 0; i < count; i++) {
		free(boats[i]);
	}
}

int main(int argc, char *argv[]) {

	// Checks for filename after executable
	if (argc != 2) {
		fprintf(stderr, "Usage: %s BoatData.csv\n", argv[0]);
		return 1;
	}

	// Initialize Boat array and load data
	Boat *boats[MAX_BOATS] = {0};
	int boat_count = loadBoats(argv[1], boats);

	printf("Welcome to the Boat Management System\n");
	printf("-------------------------------------\n");

	char option[10];

	// Main program loop
	while (1) {
		printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
		fgets(option, sizeof(option), stdin);

		switch (tolower(option[0])) {
			case 'i':
				// Show current inventory of boats
				printInventory(boats, boat_count);
				break;
			case 'a': {
				// Add new boat 
				if (boat_count >= MAX_BOATS) {
					printf("Cannot add more boats\n");
					break;
				}
				char csv_line[256];
				printf("Please enter the boat data in CSV format                 : ");
				fgets(csv_line, sizeof(csv_line), stdin);
				int len = strlen(csv_line);
				if (len > 0 && csv_line[len - 1] == '\n') {
					csv_line[len - 1] = '\0';
				}

				Boat *new_boat = createBoatCSV(csv_line);
				if (new_boat) {
					boat_count = insertBoat(boats, boat_count, new_boat);
				} else {
					printf("Invalid input\n");
				}
				break;
			}
			case 'r': {
				// Remove boat
				char input_name[MAX_NAME_LENGTH];
				printf("Please enter the boat name                               : ");
				fgets(input_name, sizeof(input_name), stdin);
				int len = strlen(input_name);
				if (len > 0 && input_name[len - 1] == '\n') {
					input_name[len - 1] = '\0';
				}

				int index = findBoat(boats, boat_count, input_name);
				if (index == -1) {
					printf("No boat with that name\n");
				} else {
					free(boats[index]);
					for (int i = index; i < boat_count - 1; i++) {
						boats[i] = boats[i + 1];
					}
					boat_count--;
				}
				break;
			}
			case 'p': {
				// Payment for specific boat
				char input_name[MAX_NAME_LENGTH];
				printf("Please enter the boat name                               : ");
				fgets(input_name, sizeof(input_name), stdin);
				int len = strlen(input_name);
				if (len > 0 && input_name[len - 1] == '\n') {
					input_name[len - 1] = '\0';
				}

				int index = findBoat(boats, boat_count, input_name);
				if (index == -1) {
					printf("No boat with that name\n");
					break;
				}

				Boat *b = boats[index];
				double payment;
				printf("Please enter the amount to be paid                       : ");
				if (scanf("%lf", &payment) != 1) {
					printf("Invalid input\n");
					while (getchar() != '\n');
					break;
				}

				while (getchar() != '\n');

				if (payment > b->amount_owed) {
					printf("That is more than the amount owed, $%.2f\n", b->amount_owed);
				} else {
					b->amount_owed -= payment;
				}
				break;
			}
			case 'm': {
				// Monthly charge for all boats based on type of storage and length
				for (int i = 0; i < boat_count; i++) {
					Boat *b = boats[i];
					double rate = 0.0;

					switch (b->type) {
						case slip: {
							rate = SLIP_RATE;
							break;
						}
						case land: {
							rate = LAND_RATE;
							break;
						}
						case trailor: {
							rate = TRAILOR_RATE;
							break;
						}
						case storage: {
							rate = STORAGE_RATE;
							break;
						}
					}
					b->amount_owed += b->length * rate;
				}
				break;
			}
			case 'x':
				// Save and exit program
				printf("Exiting the Boat Management System\n");
				saveBoats(argv[1], boats, boat_count);
				freeBoats(boats, boat_count);
				return 0;
			default:
				// Checks for invalid menu option
				printf("Invalid option %c\n", option[0]);
		}
	}

}
