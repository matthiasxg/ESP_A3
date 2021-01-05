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

//bit masks
#define CHECK_TOP 0x80 //0b1000000
#define CHECK_LEFT 0x20 //0b00100000
#define CHECK_BOTTOM 0x8 //0b00001000
#define CHECK_RIGHT 0x2 //0b00000010

#define ONE_BYTE_BIT 8

//strings
#define MAGIC_NUMBER "ESPipes"


//enums
typedef enum _Direction_
{
  TOP = 0,
  LEFT = 1,
  BOTTOM = 2,
  RIGHT = 3
} Direction;

//structs
typedef struct _Highscore_
{
  unsigned int score_;
  char name_[SIZE_HIGHSCORE_ENTRY_NAME];
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
Highscore* getHighscores(FILE *fp, unsigned int highscore_entries);
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
void getHighscoreName(char *name);
void handleHighscore(Highscore *highscores, unsigned int highscore_entries, unsigned int round);
uint8_t** getMap(FILE *fp, uint8_t width, uint8_t height);
FILE* getFilePointer(int argc, char *argv[]);
void printHighscore(Highscore *highscores, unsigned int highscore_entries);
void writeHighscore(char *path, Highscore *highscores, unsigned int highscore_entries);


//-----------------------------------------------------------------------------
///
/// The main program:
/// open/reads the file from given input, starts the game
/// and handles the highscore list
/// 
/// @param argc must be two (adress, path)
/// @param argv path of config
///
/// @return always zero
//
int main(int argc, char *argv[])
{
  FILE *fp = getFilePointer(argc, argv);
  if (fp)
  {
    uint8_t width = 0;
    uint8_t height = 0;
    uint8_t start[2] = {0};
    uint8_t end[2] = {0};
    unsigned int highscore_entries = 0;
    unsigned int round = 1;
    bool solved = false;

    getGameSpecifications(fp, &width, &height, start, end, &highscore_entries);
    Highscore *highscores = getHighscores(fp, highscore_entries);
    uint8_t **map = getMap(fp, width, height);

    if (highscores && map)
    {
      printMap(map, width, height, start, end);
      startGame(map, width, height, start, end, &round, &solved);

      if(solved)
      {
        printf(INFO_PUZZLE_SOLVED);
        printf(INFO_SCORE, round);
        handleHighscore(highscores, highscore_entries, round);
        writeHighscore(argv[1], highscores, highscore_entries);
      }
      if(round == 0)
      {
        main(argc, argv);
      }

      freeMap(map, height);
      free(highscores);
    }
  }
  return 0;
}


//------------------------------------------------------------------------------
///
/// Opens config file, creates file pointer, checks for valid
/// config file and returns the file pointer
///
/// @param argc must be two (adress, path)
/// @param argv path of config
///
/// @return file pointer of the given config
//
FILE* getFilePointer(int argc, char *argv[])
{
  if (--argc == COMMANDLINE_PARAMETER)
  {
    FILE *fp = fopen(argv[1], "rb");
    if (fp)
    {
      if (checkConfigFile(fp))
      {
        return fp;
      }
      else
      {
        printf(ERROR_INVALID_FILE, argv[1]);
        return NULL;
      }
    }
    else
    {
      printf(ERROR_OPEN_FILE, argv[1]);
      return NULL;
    }
  }
  else
  {
    printf(USAGE_APPLICATION);
    return NULL;
  }
}


//------------------------------------------------------------------------------
///
/// Allocates storage for the map, fills the map
///
/// @param fp file pointer of config file
/// @param width width of game field
/// @param height height of game field
///
/// @return the filled game field
//
uint8_t** getMap(FILE *fp, uint8_t width, uint8_t height)
{
  uint8_t **map = malloc(sizeof(uint8_t *) * height);
  if (map)
  {
    if(fillGameMap(map, fp, width, height))
    {
      fclose(fp);
      return map;
    }
    fclose(fp);
    return NULL;
  }
  else
  {
    printf(ERROR_OUT_OF_MEMORY);
    fclose(fp);
    return NULL;
  } 
}


//------------------------------------------------------------------------------
///
/// Gets the command from the command line and parses it
///
/// @param cmd pointer of Command
/// @param dir dir if cmd is rotate
/// @param row row of pipe if rotate
/// @param col col of pipe if rotate
/// @param round game round
//
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


//------------------------------------------------------------------------------
///
/// Sets a bit to 1 
///
/// @param pipe pipe of game field
/// @param index index where to set bit 1
//
void setBit(uint8_t *pipe, uint8_t index)
{
  *pipe |= (CHECK_TOP >> index);
}


//------------------------------------------------------------------------------
///
/// Clears one bit, so makes it zero
///
/// @param pipe pipe of game field
/// @param index index where to set bit 0
//
void clearBit(uint8_t *pipe, uint8_t index)
{
  *pipe &= ~(CHECK_TOP >> index);
}


//------------------------------------------------------------------------------
///
/// Returns the bit on given index
///
/// @param pipe pipe of game field
/// @param index index to read bit
///
/// @return bit of given pipe on index
//
uint8_t getBit(uint8_t pipe, uint8_t index)
{
  int bits = sizeof(pipe) * ONE_BYTE_BIT;
  index = (bits - (index + 1));
  uint8_t helper = CHECK_RIGHT >> 1;
  return ((pipe >> index) & helper);
}


//------------------------------------------------------------------------------
///
/// Starts the pipe game, manages the game
///
/// @param map the game field
/// @param width width of game field
/// @param height height of game field
/// @param start_pipe start pipe coords in game field
/// @param end_pipe end pipe coords in game field
/// @param round game round counter
/// @param solved bool if game is solved
//
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
  else if (cmd == RESTART)
  {
    *round = 0;
    return;
  }
}


//------------------------------------------------------------------------------
///
/// Rotates the pipe in given direction
///
/// @param map the game field
/// @param row row of pipe to rotate
/// @param col col of pipe to rotate
/// @param dir to rotate
//
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


//------------------------------------------------------------------------------
///
/// Checks if the given rotations values from command line are valid
/// For example if its the start/end pipe
///
/// @param row row of pipe to rotate
/// @param col col of pipe to rotate
/// @param width width of game field
/// @param height height of game field
/// @param start_pipe start pipe coords in game field
/// @param end_pipe end pipe coords in game field
///
/// @return true if values are valid, else returns false
//
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


//------------------------------------------------------------------------------
///
/// Fills the map with the values from the config file
///
/// @param map the game field
/// @param fp file pointer of config file
/// @param width width of game field
/// @param height height of game field
///
/// @return true on success, else false (Out of memory)
//
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


//------------------------------------------------------------------------------
///
/// Frees the allocated memory of the map
///
/// @param map the game field
/// @param rows count of rows of the game field
//
void freeMap(uint8_t **map, uint8_t rows)
{
  for (int row = 0; row < rows; row++)
  {
    free(map[row]);
  }
  free(map);
}


//------------------------------------------------------------------------------
///
/// Allocates memory for highscore, fills the array with values from config
///
/// @param fp file pointer of config file
/// @param highscore_entries count of highscore entries in config file
///
/// @return highscores array
//
Highscore* getHighscores(FILE *fp, unsigned int highscore_entries)
{
  Highscore *highscores = malloc(sizeof(Highscore) * highscore_entries);
  if (highscores)
  {
    for (unsigned int index = 0; index < highscore_entries; index++)
    {
      highscores[index].score_ = 0;
      strcpy(highscores[index].name_, "---");
      fread(&(highscores[index].score_), SIZE_HIGHSCORE_ENTRY_POINTS, 1, fp);
      fread(&(highscores[index].name_), SIZE_HIGHSCORE_ENTRY_NAME, 1, fp);
    }
    return highscores;
  }
  else
  {
    printf(ERROR_OUT_OF_MEMORY);
    return NULL;
  }
}


//------------------------------------------------------------------------------
///
/// Reads game specifications values from config value
///
/// @param fp file pointer of config file
/// @param width width of game field
/// @param height height of game field
/// @param start_pipe start pipe coords in game field
/// @param end_pipe end pipe coords in game field
/// @param highscore_entries count of highscore entries in config file
//
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


//------------------------------------------------------------------------------
///
/// Checks for the magic value in the config file
///
/// @param fp file pointer of config file
///
/// @return true if magic word is correct, else false
//
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


//------------------------------------------------------------------------------
///
/// Checks if given direction of pipe is out of map
///
/// @param width width of game field
/// @param height height of game field
/// @param coord of pipe
/// @param dir direction to check
///
/// @return true of directions is out of map, else false
//
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


//------------------------------------------------------------------------------
///
/// Checks if pipe is open in given direction
///
/// @param map the game field
/// @param coord of pipe
/// @param dir direction to check
///
/// @return true if pipe is open in dir, else false
//
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


//------------------------------------------------------------------------------
///
/// Checks if pipe should connect with another pipe in given direction
///
/// @param map the game field
/// @param width width of game field
/// @param height height of game field
/// @param coord coordinates of pipe on game field
/// @param dir direction to check
///
/// @return true if pipe should connect, else false
//
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


//------------------------------------------------------------------------------
///
/// Get the coords of the adjacent pipe in given direction
///
/// @param width width of game field
/// @param height height of game field
/// @param coord coordinates of given pipe
/// @param adjacent_coord storage for result coordinates
/// @param dir direction to check
///
/// @return true if success, else false
//
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


//------------------------------------------------------------------------------
///
/// Gets pointer of adjacent pipe in given direction
///
/// @param width width of game field
/// @param height height of game field
/// @param coord coordinates of given pipe
/// @param dir direction to check
///
/// @return pointer of adjacent pipe, if not found NULL
//
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


//------------------------------------------------------------------------------
///
/// Goes through all directions and updates the connection bit
///
/// @param map the game field
/// @param width width of game field
/// @param height height of game field
/// @param row row of pipe
/// @param col column of pipe
//
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


//------------------------------------------------------------------------------
///
/// If new highscore, asks user for name
///
/// @param name char array to storage users name
//
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


//------------------------------------------------------------------------------
///
/// Iterates through the highscore list and looks if the user beat
/// beats the highscore and updates the list
///
/// @param highscores list of highscores
/// @param highscore_entries  count of highscore entries in config file
/// @param round rounds user needed to solve the puzzle
//
void handleHighscore(Highscore *highscores, unsigned int highscore_entries, unsigned int round)
{
  bool done = false;
  for (size_t index = 0; index < highscore_entries; index++)
  {
    if (highscores[index].score_ > round && done == false)
    {
      printf(INFO_BEAT_HIGHSCORE);
      char name[SIZE_HIGHSCORE_ENTRY_NAME + 1] = {0};
      getHighscoreName(name);

      for (size_t move_index = highscore_entries - 1; move_index > index; move_index--)
      {
        highscores[move_index] = highscores[move_index - 1];
      }
      strcpy(highscores[index].name_, name);
      highscores[index].score_ = round;
      done = true;
    }
    else if (highscores[index].score_ == round && done == false)
    {
      while (highscores[index].score_ == round && index < highscore_entries - 1)
      {
        index++;
      }
      if (index == highscore_entries - 1 && highscores[index].score_ == round)
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

        strcpy(highscores[index].name_, name);
        highscores[index].score_ = round;
        done = true;
      }
    }
  }
  printHighscore(highscores, highscore_entries);
}


//------------------------------------------------------------------------------
///
/// Prints the highscore array after updating it
///
/// @param highscores list of highscores
/// @param highscore_entries  count of highscore entries in config file
//
void printHighscore(Highscore *highscores, unsigned int highscore_entries)
{
  printf(INFO_HIGHSCORE_HEADER);
  for (size_t index = 0; index < highscore_entries; index++)
  {
    if (highscores[index].score_ == 0)
    {
      printf(INFO_HIGHSCORE_ENTRY, "---", 0);
    }
    else
    {
      printf(INFO_HIGHSCORE_ENTRY, highscores[index].name_, highscores[index].score_);
    }
  }
}


//------------------------------------------------------------------------------
///
/// Writes the new highscore array to the config file
///
/// @param path path to config file
/// @param highscores list of highscores
/// @param highscore_entries  count of highscore entries in config file
//
void writeHighscore(char *path, Highscore *highscores, unsigned int highscore_entries)
{
  FILE *fp = fopen(path, "rb+");
  if (fp)
  {
    if (checkConfigFile(fp))
    {
      int bytes_to_highscore = SIZE_FIELD_HEIGHT + SIZE_FIELD_WIDTH + SIZE_ROW_START + 
                               SIZE_COLUMN_START + SIZE_ROW_END + SIZE_COLUMN_END + SIZE_HIGHSCORE_ENTRY;
                              
      fseek(fp, bytes_to_highscore, SEEK_CUR);
      for (size_t index = 0; index < highscore_entries; index++)
      {
        fwrite(&(highscores[index].score_), SIZE_HIGHSCORE_ENTRY_POINTS, 1, fp);
        fwrite(&(highscores[index].name_), SIZE_HIGHSCORE_ENTRY_NAME, 1, fp);
      }
      fclose(fp);
    }
    else
    {
      printf(ERROR_INVALID_FILE, path);
    }
  }
  else
  {
    printf(ERROR_OPEN_FILE, path);
  }
}
