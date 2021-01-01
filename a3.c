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
#define SIZE_HIGHSCORE_ENTRY 1
#define SIZE_GAMEFIELD_ENTRY 1

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

bool checkConfigFile(FILE *fp);

void getGameSpecifications(FILE *fp, uint8_t *size_field, uint8_t *size_start, uint8_t *size_end,
                           unsigned int *highscore_entries);

void getHighscores(FILE *fp, unsigned int highscore_entries, Highscore *highscores);

bool fillGameMap(uint8_t **map, FILE *fp, uint8_t *size_field);

void freeMap(uint8_t **map, uint8_t rows);

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
    FILE *fp = fopen(argv[1], "rb");
    if (fp)
    {
      if (checkConfigFile(fp))
      {
        uint8_t size_field[2] = {0};
        uint8_t size_start[2] = {0};
        uint8_t size_end[2] = {0};
        unsigned int highscore_entries = 0;
        getGameSpecifications(fp, size_field, size_start, size_end, &highscore_entries);

        Highscore *highscores = malloc(sizeof(Highscore) * highscore_entries);
        if (highscores)
        {
          getHighscores(fp, highscore_entries, highscores);

          uint8_t **map = malloc(sizeof(uint8_t *) * size_field[1]);
          if (map)
          {
            if (fillGameMap(map, fp, size_field)) {
              printMap(map, size_field[0], size_field[1], size_start, size_end);
            }
          }
          else
          {
            printf(ERROR_OUT_OF_MEMORY);
          }
        }
        else
        {
          printf(ERROR_OUT_OF_MEMORY);
        }

        /*
        for (unsigned int i = 0; i < highscore_entries; i++)
        {
          printf("Name: %s - Score: %d\n", highscores[i].name, highscores[i].score);
        }

        printf("Field: %d - %d\n", size_field[0], size_field[1]);
        printf("Start: %d - %d\n", size_start[0], size_start[1]);
        printf("End: %d - %d\n", size_end[0], size_end[1]);
        printf("Highscores: %d\n", highscore_entries);*/
      }
      else
      {
        printf(ERROR_INVALID_FILE, argv[1]);
      }
    }
    else
    {
      printf(ERROR_OPEN_FILE, argv[1]);
    }
  }
  else
  {
    printf(USAGE_APPLICATION);
  }
  return 0;
}

bool fillGameMap(uint8_t **map, FILE *fp, uint8_t *size_field)
{
  for (int row = 0; row < size_field[1]; row++)
  {
    map[row] = malloc(sizeof(uint8_t) * size_field[0]);
    if (map[row]) {
      for (int column = 0; column < size_field[0]; column++)
      {
        fread(&(map[row][column]), SIZE_GAMEFIELD_ENTRY, 1, fp);
      }
    }
    else
    {
      printf(ERROR_OUT_OF_MEMORY);
      freeMap(map, size_field[1]);
      return false;
    }
  }
  return true;
}

void freeMap(uint8_t **map, uint8_t rows) {
  for (int row = 0; row < rows; row++) {
    free(map[row]);
  }
  free(map);
}


void getHighscores(FILE *fp, unsigned int highscore_entries, Highscore *highscores)
{
  for (unsigned int index = 0; index < highscore_entries; index++)
  {
    fread(&(highscores[index].score), SIZE_HIGHSCORE_ENTRY_POINTS, 1, fp);
    fread(&(highscores[index].name), SIZE_HIGHSCORE_ENTRY_NAME, 1, fp);
  }
}

// TODO
void getGameSpecifications(FILE *fp, uint8_t *size_field, uint8_t *size_start, uint8_t *size_end, unsigned int
*highscore_entries)
{
  fread(size_field, SIZE_FIELD_WIDTH, 1, fp);
  fread(size_field + 1, SIZE_FIELD_HEIGHT, 1, fp);

  fread(size_start, SIZE_ROW_START, 1, fp);
  fread(size_start + 1, SIZE_COLUMN_START, 1, fp);

  fread(size_end, SIZE_ROW_END, 1, fp);
  fread(size_end + 1, SIZE_COLUMN_END, 1, fp);

  fread(highscore_entries, SIZE_HIGHSCORE_ENTRY, 1, fp);
}

// TODO
bool checkConfigFile(FILE *fp)
{
  char magic_number[SIZE_MAGIC_NUMBER + 1] = {0};
  fread(magic_number, sizeof(char), SIZE_MAGIC_NUMBER, fp);
  if (strcmp(magic_number, MAGIC_NUMBER) != 0)
  {
    return false;
  }
  return true;
}
