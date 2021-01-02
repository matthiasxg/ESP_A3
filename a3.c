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

void getGameSpecifications(FILE *fp, uint8_t *width, uint8_t *height, uint8_t *start_pipe, uint8_t *end_pipe,
                           unsigned int *highscore_entries);

void getHighscores(FILE *fp, unsigned int highscore_entries, Highscore *highscores);

bool fillGameMap(uint8_t **map, FILE *fp, uint8_t width, uint8_t height);

void freeMap(uint8_t **map, uint8_t rows);

void startGame(uint8_t **map, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe, unsigned int
round);

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
        uint8_t width = 0;
        uint8_t height = 0;
        uint8_t start_pipe[2] = {0};
        uint8_t end_pipe[2] = {0};
        unsigned int highscore_entries = 0;
        getGameSpecifications(fp, &width, &height, start_pipe, end_pipe, &highscore_entries);

        Highscore *highscores = malloc(sizeof(Highscore) * highscore_entries);
        if (highscores)
        {
          getHighscores(fp, highscore_entries, highscores);

          uint8_t **map = malloc(sizeof(uint8_t *) * height);

          if (fillGameMap(map, fp, width, height))
          {
            startGame(map, width, height, start_pipe, end_pipe, 1);
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

void startGame(uint8_t **map, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe, unsigned int round)
{
  printMap(map, width, height, start_pipe, end_pipe);
  printf(INPUT_PROMPT, round);

  char *line = getLine();
  if (line != NULL)
  {
    Command cmd = NONE;
    Direction dir = TOP;
    uint8_t row = 0;
    uint8_t col = 0;
    char *result = parseCommand(line, &cmd, (size_t *) dir, &row, &col);

    if (result == (char*) 1) {
      printf("ROTATE");
    }
    else if (result == NULL) {
      switch ((size_t) cmd)
      {
        case NONE:
          startGame(map, width, height, start_pipe, end_pipe, round);
          break;
        case HELP:
          printf(HELP_TEXT);
          startGame(map, width, height, start_pipe, end_pipe, round);
          break;
        case QUIT:
          return;
        case RESTART:
          break;
      }
    }
    else
    {
      printf(ERROR_UNKNOWN_COMMAND, result);
    }

    free(line);
  }
  else if (line == (char*) EOF)
  {
    return;
  }
  else
  {
    printf(ERROR_OUT_OF_MEMORY);
  }
}

bool fillGameMap(uint8_t **map, FILE *fp, uint8_t width, uint8_t height)
{
  for (int row = 0; row < height; row++)
  {
    map[row] = malloc(sizeof(uint8_t) * width);
    if (map[row])
    {
      for (int column = 0; column < width; column++)
      {
        fread(&(map[row][column]), SIZE_GAMEFIELD_ENTRY, 1, fp);
      }
    }
    else
    {
      printf(ERROR_OUT_OF_MEMORY);
      freeMap(map, height);
      return false;
    }
  }
  return true;
}

void freeMap(uint8_t **map, uint8_t rows)
{
  for (int row = 0; row < rows; row++)
  {
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
void getGameSpecifications(FILE *fp, uint8_t *width, uint8_t *height, uint8_t *start_pipe, uint8_t *end_pipe,
                           unsigned int *highscore_entries)
{
  fread(width, SIZE_FIELD_WIDTH, 1, fp);
  fread(height, SIZE_FIELD_HEIGHT, 1, fp);

  fread(start_pipe, SIZE_ROW_START, 1, fp);
  fread(start_pipe + 1, SIZE_COLUMN_START, 1, fp);

  fread(end_pipe, SIZE_ROW_END, 1, fp);
  fread(end_pipe + 1, SIZE_COLUMN_END, 1, fp);

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
