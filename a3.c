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
#include <ctype.h>
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
#define SIZE_BUFFER 300

#define SIZE_HIGHSCORE_ENTRY_POINTS 1
#define SIZE_HIGHSCORE_ENTRY_NAME 3

#define CHECK_TOP 0x80 //0b1000000
#define CHECK_LEFT 0x20 //0b00100000
#define CHECK_BOTTOM 0x8 //0b00001000
#define CHECK_RIGHT 0x2 //0b00000010

#define ONE_BYTE_BIT 8

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
  char name[SIZE_HIGHSCORE_ENTRY_NAME + 1];
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
void startGame(uint8_t **map, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe, unsigned int *round, bool *solved);
void getCommand(Command *cmd, Direction *dir, uint8_t *row, uint8_t *col, unsigned int round);
bool checkRotateValues(uint8_t row, uint8_t col, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe);
void rotatePipe(uint8_t **map, uint8_t row, uint8_t col, Direction dir);
void setBit(uint8_t *pipe, uint8_t index);
uint8_t getBit(uint8_t pipe, uint8_t index);
void clearBit(uint8_t *pipe, uint8_t index);
void updateConnections(uint8_t **map, uint8_t width, uint8_t height, uint8_t row, uint8_t col);
bool checkHighscore(Highscore *highscores, unsigned int highscore_entries, unsigned int round);
void getHighscoreName(char *name);
void handleHighscore(Highscore *highscores, unsigned int highscore_entries, unsigned int round);

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
          if (map) {
            if (fillGameMap(map, fp, width, height))
            {
              fclose(fp);
              printMap(map, width, height, start_pipe, end_pipe);
              unsigned int round = 1;
              bool solved = false;
              startGame(map, width, height, start_pipe, end_pipe, &round, &solved);
                if(solved) {
                  printf(INFO_PUZZLE_SOLVED);
                  printf(INFO_SCORE, round);
                  handleHighscore(highscores, highscore_entries, round);
                }
              freeMap(map, height);
              free(highscores);
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

void setBit(uint8_t *pipe, uint8_t index)
{
  *pipe |= (CHECK_TOP >> index);
}

void clearBit(uint8_t *pipe, uint8_t index)
{
  *pipe &= ~(CHECK_TOP >> index);
}

uint8_t getBit(uint8_t pipe, uint8_t index)
{
  int bits = sizeof(pipe) * ONE_BYTE_BIT;
  index = (bits - (index + 1));
  uint8_t helper = CHECK_RIGHT >> 1;
  return ((pipe >> index) & helper);
}

void startGame(uint8_t **map, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe, unsigned int *round, bool *solved)
{
  Command cmd = NONE;
  Direction dir = TOP;
  uint8_t row = 0;
  uint8_t col = 0;
  getCommand(&cmd, &dir, &row, &col, *round);
  if (cmd == ROTATE)
  {
    --row;
    --col;
    if (checkRotateValues(row, col, width, height, start_pipe, end_pipe))
    {
      rotatePipe(map, row, col, dir);
      updateConnections(map, width, height, row, col);
      printMap(map, width, height, start_pipe, end_pipe);
      if (arePipesConnected(map, width, height, start_pipe, end_pipe))
      {
        *solved = true;
      }
      else
      {
        *round = (*round) + 1;
        startGame(map, width, height, start_pipe, end_pipe, round, solved);
      }
    }
    else
    {
      startGame(map, width, height, start_pipe, end_pipe, round, solved);
    }
  }
}

void rotatePipe(uint8_t **map, uint8_t row, uint8_t col, Direction dir)
{
  uint8_t *pipe = (map[row]) + col;
  uint8_t first_bit = 0;
  uint8_t second_bit = 0;
  switch (dir)
  {
  case LEFT:
    first_bit = getBit(*pipe, 6);
    second_bit = getBit(*pipe, 7);
    *pipe >>= 2;
    if (first_bit)
    {
      setBit(pipe, 0);
    }
    if (second_bit)
    {
      setBit(pipe, 1);
    }
    break;
  
  case RIGHT:
    first_bit = getBit(*pipe, 0);
    second_bit = getBit(*pipe, 1);
    *pipe <<= 2;
    if (first_bit)
    {
      setBit(pipe, 6);
    }
    if (second_bit)
    {
      setBit(pipe, 7);
    }
    break;
  default:
    break;
  }
}

bool checkRotateValues(uint8_t row, uint8_t col, uint8_t width, uint8_t height, uint8_t *start_pipe, uint8_t *end_pipe)
{
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
      Direction adjacent_dir = TOP;
      switch (dir)
      {
      case TOP:
        adjacent_dir = BOTTOM;
        break;
      case LEFT:
        adjacent_dir = RIGHT;
        break;
      case BOTTOM:
        adjacent_dir = TOP;
        break;
      case RIGHT:
        adjacent_dir = LEFT;
        break;
      }
      if (isPipeOpenInDirection(map, adjacent_coord, adjacent_dir))
      {
        setBit(&map[adjacent_coord[0]][adjacent_coord[1]], (2 * (adjacent_dir + 1)) - 1);
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

void updateConnections(uint8_t **map, uint8_t width, uint8_t height, uint8_t row, uint8_t col)
{
  uint8_t coords[2] = {row, col};
  for (Direction dir = 0; dir <= RIGHT; dir++)
  {
    if(shouldPipeConnectInDirection(map, width, height, coords, dir))
    {
      setBit(&map[row][col], (2 * (dir + 1)) - 1);
    }
    else
    {
      clearBit(&map[row][col], (2 * (dir + 1)) - 1);
    }
  }
}

bool checkHighscore(Highscore *highscores, unsigned int highscore_entries, unsigned int round)
{
  for (size_t index = 0; index < highscore_entries; index++)
  {
    if (highscores[index].score < round)
    {
      return false;
    }
  }
  return true;
}

void getHighscoreName(char *name)
{
  printf(INPUT_NAME);
  char input[SIZE_BUFFER] = {0};
  if(fgets(input, SIZE_BUFFER, stdin))
  {
    if(strchr(input, '\n') != NULL && strlen(input) == (SIZE_HIGHSCORE_ENTRY_NAME + 1))
    {
      bool alpha = true;
      for (size_t index = 0; index < SIZE_HIGHSCORE_ENTRY_NAME; index++)
      {
        if(isalpha(input[index]))
        {

          name[index] = toupper(input[index]);
        }
        else
        {
          alpha = false;
        }
      }
      if(!alpha)
      {
        printf(ERROR_NAME_ALPHABETIC);
        getHighscoreName(name);
      }
    }
    else
    {
      printf(ERROR_NAME_LENGTH);
      getHighscoreName(name);
    }
  }
}

void handleHighscore(Highscore *highscores, unsigned int highscore_entries, unsigned int round)
{
  bool done = false;
  for (size_t index = 0; index < highscore_entries; index++)
  {
    
    if (highscores[index].score > round && done == false)
    {
      printf(INFO_BEAT_HIGHSCORE);
      char name[SIZE_HIGHSCORE_ENTRY_NAME + 1] = {0};
      getHighscoreName(name);

      for (size_t move_index = highscore_entries - 1; move_index > index; move_index--)
      {
        highscores[move_index] = highscores[move_index - 1];
      }

      strcpy(highscores[index].name, name);
      highscores[index].score = round;

      done = true;
    }
    else if (highscores[index].score == round && done == false)
    {
      while (highscores[index].score == round && index < highscore_entries - 1)
      {
        index++;
      }
      if (index == highscore_entries - 1 && highscores[index].score == round)
      {
        break;
      }
      else
      {
        printf(INFO_BEAT_HIGHSCORE);
        char name[SIZE_HIGHSCORE_ENTRY_NAME + 1] = {0};
        getHighscoreName(name);
        for (size_t move_index = highscore_entries - 1; move_index > index; move_index--)
        {
          highscores[move_index] = highscores[move_index - 1];
        }

        strcpy(highscores[index].name, name);
        highscores[index].score = round;
        done = true;
      }
    }
  }
  
  printf(INFO_HIGHSCORE_HEADER);
  for (size_t index = 0; index < highscore_entries; index++)
  {
    if (highscores[index].score == 0)
    {
      printf(INFO_HIGHSCORE_ENTRY, "---", 0);
    }
    else
    {
      printf(INFO_HIGHSCORE_ENTRY, highscores[index].name, highscores[index].score);
    }
  }
  
}

