/*
 * This is helper and printing functions for the trains and wyse.
 */
#include <stdio.h>
#include <syscall.h>
#include <irq.h>
#include <cs452/machine/serial.h>

#include "trainlib.h"

/* The USART train port. */
#define TRAIN (USART_0_BASE)

/* Internal train commands. */
#define CMD_REVERSE (15)
#define CMD_SWITCH_STRAIGHT (33)
#define CMD_SWITCH_CURVED (34)
#define CMD_SWITCH_SOLENOIDOFF (32)
#define CMD_SWITCH_DELAY (200)
#define CMD_SENSOR_BASE (128)
#define CMD_GO (96)
#define CMD_STOP (97)

void TrainInit ()
{
  Config(TRAIN, IRQ_SERIAL0, 2400, 8, 0, 2);
}

void TrainExecCmd (TrainCommand_t *cmd)
{
  char cmdbuf[4];
  switch (cmd->type) {
    case TRAIN_CMD_FORWARD:
      cmdbuf[0] = (char)cmd->param;
      cmdbuf[1] = (char)cmd->id;
      Write(TRAIN, cmdbuf, 2);
      break;

    case TRAIN_CMD_REVERSE:
      cmdbuf[0] = (char)CMD_REVERSE;
      cmdbuf[1] = (char)cmd->id;
      cmdbuf[2] = (char)cmd->param;
      cmdbuf[3] = (char)cmd->id;
      Write(TRAIN, cmdbuf, 4);
      break;

    case TRAIN_CMD_SWITCH:
      cmdbuf[0] = (char)(cmd->param == SWITCH_STRAIGHT ? CMD_SWITCH_STRAIGHT : CMD_SWITCH_CURVED);
      cmdbuf[1] = (char)cmd->id;
      Write(TRAIN, cmdbuf, 2);
      break;

    case TRAIN_CMD_SWITCH_END:
      Delay(CMD_SWITCH_DELAY);
      Put(TRAIN, (char)CMD_SWITCH_SOLENOIDOFF);
      break;

    case TRAIN_CMD_SENSOR:
      Put(TRAIN, CMD_SENSOR_BASE + cmd->id);
      break;

    case TRAIN_CMD_FUNCTION:
      cmdbuf[0] = (char)cmd->param;
      cmdbuf[1] = (char)cmd->id;
      Write(TRAIN, cmdbuf, 2);
      break;

    case TRAIN_CMD_STOP:
      Put(TRAIN, (char)CMD_STOP);
      break;

    case TRAIN_CMD_GO:
      Put(TRAIN, (char)CMD_GO);
      break;

    case TRAIN_CMD_NOTHING:
    default:
      break;
  }
}

unsigned int TrainRead ()
{
  return Get(TRAIN) & 0xFF;
}

