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

#define CHECK_TOP 64
#define CHECK_LEFT 16
#define CHECK_BOTTOM 4
#define CHECK_RIGHT 1

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

bool isPipeOpenInDirection(uint8_t **map, uint8_t coord[2], Direction dir);

bool shouldPipeConnectInDirection(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);

uint8_t *getAdjacentPipe(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);

bool getCoordOfAdjacentPipe(uint8_t width, uint8_t height, uint8_t coord[2], uint8_t adjacent_coord[2], Direction dir);

bool checkConfigFile(FILE *fp);

void getGameSpecifications(FILE *fp, uint8_t *width, uint8_t *height, uint8_t *start_pipe, uint8_t *end_pipe,
                           unsigned int *highscore_entries);

void getHighscores(FILE *fp, unsigned int highscore_entries, Highscore *highscores);

bool fillGameMap(uint8_t **map, FILE *fp, uint8_t width, uint8_t height);

void freeMap(uint8_t **map, uint8_t rows);

void startGame(uint8_t **map, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe, unsigned int round);

void getCommand(Command *cmd, Direction *dir, uint8_t *row, uint8_t *col, unsigned int round);

bool checkRotateValues(uint8_t row, uint8_t col, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe);

void rotatePipe(uint8_t **map, uint8_t width, uint8_t height, uint8_t row, uint8_t col, Direction dir);

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
            fclose(fp);
            printMap(map, width, height, start_pipe, end_pipe);
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

void getCommand(Command *cmd, Direction *dir, uint8_t *row, uint8_t *col, unsigned int round)
{
  printf(INPUT_PROMPT, round);
  char *line = getLine();

  if (line == NULL)
  {
    printf(ERROR_OUT_OF_MEMORY);
  }
  else if (line == (char *)EOF)
  {
    return;
  }
  else
  {
    char *result = parseCommand(line, cmd, (size_t *)dir, row, col);
    if (result == (char *)1)
    {
      printf(USAGE_COMMAND_ROTATE);
      getCommand(cmd, dir, row, col, round);
    }
    else if (result == NULL)
    {
      switch ((size_t)*cmd)
      {
      case NONE:
        getCommand(cmd, dir, row, col, round);
        break;
      case HELP:
        printf(HELP_TEXT);
        getCommand(cmd, dir, row, col, round);
        break;
      case QUIT:
        break;
      case RESTART:
        // TODO
        break;
      }
    }
    else
    {
      printf(ERROR_UNKNOWN_COMMAND, result);
      getCommand(cmd, dir, row, col, round);
    }
    free(line);
  }
}

void startGame(uint8_t **map, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe, unsigned int round)
{
  Command cmd = NONE;
  Direction dir = TOP;
  uint8_t row = 0;
  uint8_t col = 0;

  getCommand(&cmd, &dir, &row, &col, round);
  if (cmd == ROTATE)
  {
    if (checkRotateValues(row, col, width, height, start_pipe, end_pipe))
    {
      //rotatePipe(map, width, height, row, col, dir);
    }
    else
    {
      startGame(map, width, height, start_pipe, end_pipe, round);
    }
  }
}

// void rotatePipe(uint8_t **map, uint8_t width, uint8_t height, uint8_t row, uint8_t col, Direction dir)
// {
// }

bool checkRotateValues(uint8_t row, uint8_t col, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe)
{
  row--;
  col--;
  if (row >= height || col >= width)
  {
    printf(USAGE_COMMAND_ROTATE);
    return false;
  }
  if (row == start_pipe[0] && col == start_pipe[1])
  {
    printf(ERROR_ROTATE_INVALID);
    return false;
  }
  if (row == end_pipe[0] && col == end_pipe[1])
  {
    printf(ERROR_ROTATE_INVALID);
    return false;
  }
  return true;
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

bool isDirectionOutOfMap(uint8_t width, uint8_t height, uint8_t coord[2], Direction dir)
{
  switch (dir)
  {
  case TOP:
    if (coord[0] == 0)
    {
      return true;
    }
    break;

  case LEFT:
    if (coord[1] == 0)
    {
      return true;
    }
    break;

  case BOTTOM:
    if (coord[0] == (height - 1))
    {
      return true;
    }
    break;

  case RIGHT:
    if (coord[1] == (width - 1))
    {
      return true;
    }
    break;
  }
  return false;
}

bool isPipeOpenInDirection(uint8_t **map, uint8_t coord[2], Direction dir)
{
  uint8_t pipe = map[coord[0]][coord[1]];
  switch (dir)
  {
  case TOP:
    if ((pipe ^ CHECK_TOP) < pipe)
    {
      return true;
    }
    break;

  case LEFT:
    if ((pipe ^ CHECK_LEFT) < pipe)
    {
      return true;
    }
    break;

  case BOTTOM:
    if ((pipe ^ CHECK_BOTTOM) < pipe)
    {
      return true;
    }
    break;

  case RIGHT:
    if ((pipe ^ CHECK_RIGHT) < pipe)
    {
      return true;
    }
    break;
  }
  return false;
}

bool shouldPipeConnectInDirection(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir)
{
  if (isDirectionOutOfMap(width, height, coord, dir))
  {
    return false;
  }
  if (isPipeOpenInDirection(map, coord, dir))
  {
    uint8_t adjacent_coord[2] = {0};
    if (getCoordOfAdjacentPipe(width, height, coord, adjacent_coord, dir))
    {
      Direction adjadent_dir = TOP;
      if (dir == TOP || dir == LEFT)
      {
        adjadent_dir += 2;
      }
      else
      {
        adjadent_dir -= 2;
      }
      if (isPipeOpenInDirection(map, adjacent_coord, adjadent_dir))
      {
        return true;
      }
    }
  }
  return false;
}

bool getCoordOfAdjacentPipe(uint8_t width, uint8_t height, uint8_t coord[2], uint8_t adjacent_coord[2], Direction dir)
{
  if (!isDirectionOutOfMap(width, height, coord, dir))
  {
    switch (dir)
    {
    case TOP:
      adjacent_coord[0] = (coord[0] - 1);
      adjacent_coord[1] = coord[1];
      return true;
      break;

    case LEFT:
      adjacent_coord[0] = coord[0];
      adjacent_coord[1] = (coord[1] - 1);
      return true;
      break;

    case BOTTOM:
      adjacent_coord[0] = (coord[0] + 1);
      adjacent_coord[1] = coord[1];
      return true;
      break;

    case RIGHT:
      adjacent_coord[0] = coord[0];
      adjacent_coord[1] = (coord[1] + 1);
      return true;
      break;
    }
  }
  return false;
}

uint8_t *getAdjacentPipe(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir)
{
  if (isDirectionOutOfMap(width, height, coord, dir))
  {
    return NULL;
  }

  switch (dir)
  {
  case TOP:
    return &map[(coord[0] - 1)][coord[1]];
    break;

  case LEFT:
    return &map[coord[0]][(coord[1] - 1)];
    break;

  case BOTTOM:
    return &map[(coord[0] + 1)][coord[1]];
    break;

  case RIGHT:
    return &map[coord[0]][(coord[1] + 1)];
    break;
  }
  return NULL;
}
