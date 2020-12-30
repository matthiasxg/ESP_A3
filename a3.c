//-----------------------------------------------------------------------------
// a3.c
//
// Pipe-minigame, rotate the pipes to reach the goal
//
// Group: 9
//
// Author: 12007662
//-----------------------------------------------------------------------------
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "framework.h"

//constants
#define COMMANDLINE_PARAMETER 1

#define SIZE_MAGIC_NUMBER 7
#define SIZE_FIELD_WIDTH 1
#define SIZE_FIELD_HEIGHT 1
#define SIZE_ROW_START 1
#define SIZE_COLUMN_START 1
#define SIZE_ROW_END 1
#define SIZE_COLUMN_END 1
#define SIZE_HIGHSCORE_ENTRIES 1

#define SIZE_HIGHSCORE_ENTRY_POINTS 1
#define SIZE_HIGHSCORE_ENTRY_NAME 3

//strings
#define MAGIC_NUMBER "ESPipes"

typedef enum _Direction_
{
  TOP = 0,
  LEFT = 1,
  BOTTOM = 2,
  RIGHT = 3
} Direction;

typedef struct _Highscore_
{
  unsigned int score;
  char name[SIZE_HIGHSCORE_ENTRY_NAME];
} Highscore;


// forward declarations
bool isDirectionOutOfMap(uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);

bool isPipeOpenInDirection(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);

bool shouldPipeConnectInDirection(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);

uint8_t *getAdjacentPipe(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);

bool checkConfigFile(char *path);

void getGameSpecifications(char *path, int *size_field, int *size_start, int *size_end, int *highscore_entries);

void getHighscores(char *path, int highscore_entries, Highscore *highscores);

//-----------------------------------------------------------------------------
///
/// The main program:
/// TODO
///
//
int main(int argc, char *argv[])
{
  if (--argc == COMMANDLINE_PARAMETER)
  {
    char *path = argv[1];
    if (checkConfigFile(path))
    {
      int size_field[2] = {0};
      int size_start[2] = {0};
      int size_end[2] = {0};
      int highscore_entries = 0;

      getGameSpecifications(path, size_field, size_start, size_end, &highscore_entries);

      Highscore *highscores = malloc(sizeof(Highscore) * highscore_entries);
      if (highscores)
      {
        getHighscores(path, highscore_entries, highscores);
      }
      else
      {
        printf(ERROR_OUT_OF_MEMORY);
      }

      /*for (int i = 0; i < highscore_entries; i++)
      {
        printf("Name: %s - Score: %d\n", highscores[i].name, highscores[i].score);
      }

      printf("Field: %d - %d\n", size_field[0], size_field[1]);
      printf("Start: %d - %d\n", size_start[0], size_start[1]);
      printf("End: %d - %d\n", size_end[0], size_end[1]);
      printf("Highscores: %d\n", highscore_entries);*/
    }
  }
  else
  {
    printf(USAGE_APPLICATION);
  }
  return 0;
}

void getHighscores(char *path, int highscore_entries, Highscore *highscores)
{
  FILE *fp = fopen(path, "rb");
  if (fp)
  {
    fseek(fp, 14, SEEK_SET);

    for (int index = 0; index < highscore_entries; index++)
    {
      fread(&(highscores[index].score), SIZE_HIGHSCORE_ENTRY_POINTS, 1, fp);
      fread(&(highscores[index].name), SIZE_HIGHSCORE_ENTRY_NAME, 1, fp);
    }

    fclose(fp);
  }
  else
  {
    printf(ERROR_OPEN_FILE, path);
  }
}

// TODO
void getGameSpecifications(char *path, int *size_field, int *size_start, int *size_end, int *highscore_entries)
{
  FILE *fp = fopen(path, "rb");
  if (fp)
  {
    fseek(fp, SIZE_MAGIC_NUMBER, SEEK_SET);

    fread(size_field, SIZE_FIELD_WIDTH, 1, fp);
    fread(size_field + 1, SIZE_FIELD_HEIGHT, 1, fp);

    fread(size_start, SIZE_ROW_START, 1, fp);
    fread(size_start + 1, SIZE_COLUMN_START, 1, fp);

    fread(size_end, SIZE_ROW_END, 1, fp);
    fread(size_end + 1, SIZE_COLUMN_END, 1, fp);

    fread(highscore_entries, SIZE_HIGHSCORE_ENTRIES, 1, fp);

    fclose(fp);
  }
  else
  {
    printf(ERROR_OPEN_FILE, path);
  }
}

// TODO
bool checkConfigFile(char *path)
{
  FILE *fp = fopen(path, "rb");
  if (fp)
  {
    char *magic_number = malloc(sizeof(char) * SIZE_MAGIC_NUMBER);
    if (magic_number)
    {
      fread(magic_number, sizeof(char), SIZE_MAGIC_NUMBER, fp);
      if (strcmp(magic_number, MAGIC_NUMBER) != 0)
      {
        printf(ERROR_INVALID_FILE, path);
        free(magic_number);
        fclose(fp);
        return false;
      }
      free(magic_number);
    }
    else
    {
      printf(ERROR_OUT_OF_MEMORY);
      fclose(fp);
      return false;
    }
  }
  else
  {
    printf(ERROR_OPEN_FILE, path);
    return false;
  }
  fclose(fp);
  return true;
}
