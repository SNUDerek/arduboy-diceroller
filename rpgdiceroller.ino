// created by Derek Hommel, 2026
// MIT licensed
// Simple RPG Dice Roller
// for solo role-playing w/o dice
// made with AI assistance 

#include <Arduboy2.h>

Arduboy2 arduboy;

enum AppScreen {
  SCREEN_START,
  SCREEN_INPUT,
  SCREEN_TRANSITION,
  SCREEN_RESULTS
};

enum RollMode {
  MODE_DICE,
  MODE_PBTA,
  MODE_IRONSWORN
};

enum HitResult {
  HIT_MISS,
  HIT_WEAK,
  HIT_STRONG
};

const uint8_t SCREEN_W = 128;
const uint8_t SCREEN_H = 64;

const uint8_t CHAR_W = 6;
const uint8_t CHAR_H = 8;

const uint8_t DICE_SIDES[] = {4, 6, 8, 10, 12, 20, 100};
const uint8_t DICE_SIDE_COUNT = sizeof(DICE_SIDES) / sizeof(DICE_SIDES[0]);

AppScreen screen = SCREEN_START;
RollMode mode = MODE_DICE;

uint8_t diceCount = 1;
uint8_t diceSideIndex = 1; // default d6
int8_t modifier = 0;

uint8_t diceResults[20];
uint16_t diceTotal = 0;

uint8_t rollA = 0;
uint8_t rollB = 0;
uint8_t rollC = 0;
int16_t rollSum = 0;
HitResult hitResult = HIT_MISS;
bool match = false;

uint32_t transitionStartMs = 0;
uint8_t transitionDots = 0;

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(30);
  arduboy.initRandomSeed();
}

void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  arduboy.pollButtons();

  arduboy.clear();

  switch (screen) {
    case SCREEN_START:
      updateStart();
      drawStart();
      break;

    case SCREEN_INPUT:
      updateInput();
      drawInput();
      break;

    case SCREEN_TRANSITION:
      updateTransition();
      drawTransition();
      break;

    case SCREEN_RESULTS:
      updateResults();
      drawResults();
      break;
  }

  arduboy.display();
}

void updateStart() {
  if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
    screen = SCREEN_INPUT;
  }
}

void updateInput() {
  if (arduboy.justPressed(A_BUTTON)) {
    mode = (RollMode)((mode + 1) % 3);
    modifier = 0;
  }

  if (arduboy.justPressed(B_BUTTON)) {
    doRoll();
    screen = SCREEN_TRANSITION;
    transitionStartMs = millis();
    transitionDots = 0;
    return;
  }

  if (mode == MODE_DICE) {
    if (arduboy.justPressed(UP_BUTTON) && diceCount < 20) {
      diceCount++;
    }

    if (arduboy.justPressed(DOWN_BUTTON) && diceCount > 1) {
      diceCount--;
    }

    if (arduboy.justPressed(RIGHT_BUTTON) && diceSideIndex < DICE_SIDE_COUNT - 1) {
      diceSideIndex++;
    }

    if (arduboy.justPressed(LEFT_BUTTON) && diceSideIndex > 0) {
      diceSideIndex--;
    }
  } else {
    if (arduboy.justPressed(UP_BUTTON) && modifier < 10) {
      modifier++;
    }

    if (arduboy.justPressed(DOWN_BUTTON) && modifier > -10) {
      modifier--;
    }

    if (arduboy.justPressed(RIGHT_BUTTON) && modifier < 10) {
      modifier++;
    }

    if (arduboy.justPressed(LEFT_BUTTON) && modifier > -10) {
      modifier--;
    }
  }
}

void updateTransition() {
  uint32_t elapsed = millis() - transitionStartMs;

  transitionDots = elapsed / 200;
  if (transitionDots > 4) {
    transitionDots = 4;
  }

  if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON) || elapsed >= 1000) {
    screen = SCREEN_RESULTS;
  }
}

void updateResults() {
  if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
    if (mode == MODE_PBTA || mode == MODE_IRONSWORN) {
      modifier = 0;
    }

    screen = SCREEN_INPUT;
  }
}

void doRoll() {
  diceTotal = 0;
  rollA = 0;
  rollB = 0;
  rollC = 0;
  rollSum = 0;
  match = false;
  hitResult = HIT_MISS;

  if (mode == MODE_DICE) {
    uint8_t sides = DICE_SIDES[diceSideIndex];

    for (uint8_t i = 0; i < diceCount; i++) {
      diceResults[i] = random(1, sides + 1);
      diceTotal += diceResults[i];
    }
  } else if (mode == MODE_PBTA) {
    rollA = random(1, 7);
    rollB = random(1, 7);
    rollSum = rollA + rollB + modifier;

    match = (rollA == rollB);

    if (rollSum <= 6) {
      hitResult = HIT_MISS;
    } else if (rollSum <= 9) {
      hitResult = HIT_WEAK;
    } else {
      hitResult = HIT_STRONG;
    }
  } else if (mode == MODE_IRONSWORN) {
    rollA = random(1, 7);   // action die
    rollB = random(1, 11);  // challenge die
    rollC = random(1, 11);  // challenge die

    if (rollB > rollC) {
      uint8_t tmp = rollB;
      rollB = rollC;
      rollC = tmp;
    }

    rollSum = rollA + modifier;
    match = (rollB == rollC);

    uint8_t successes = 0;
    if (rollSum > rollB) successes++;
    if (rollSum > rollC) successes++;

    if (successes == 0) {
      hitResult = HIT_MISS;
    } else if (successes == 1) {
      hitResult = HIT_WEAK;
    } else {
      hitResult = HIT_STRONG;
    }
  }
}

void drawStart() {
  arduboy.setTextColor(WHITE);

  arduboy.setTextSize(2);
  drawCenteredText("RPG", 4, 2);

  arduboy.setTextSize(1);
  drawCenteredText("DICE ROLLER", 24, 1);
  drawCenteredText("DPAD: adjust #s", 42, 1);
  drawCenteredText("A: Mode B: Roll", 54, 1);
}

void drawInput() {
  if (mode == MODE_DICE) {
    drawDiceInput();
  } else if (mode == MODE_PBTA) {
    drawModifierInput("PbtA");
  } else {
    drawModifierInput("Ironsworn");
  }
}

void drawDiceInput() {
  arduboy.setTextSize(2);
  arduboy.setTextColor(WHITE);

  const int16_t centerX = 64;
  const int16_t y = 24;

  char leftBuf[4];
  char rightBuf[5];

  snprintf(leftBuf, sizeof(leftBuf), "%2u", diceCount);
  snprintf(rightBuf, sizeof(rightBuf), "%u", DICE_SIDES[diceSideIndex]);

  // 2x font: 12px wide, 16px high.
  // Keep the "d" fixed around screen center.
  int16_t dX = centerX - 6;
  int16_t leftX = dX - 6 - (2 * CHAR_W * 2);
  int16_t rightX = dX + (CHAR_W * 2) + 6;

  arduboy.setCursor(leftX, y);
  arduboy.print(leftBuf);

  arduboy.setCursor(dX, y);
  arduboy.print("d");

  arduboy.setCursor(rightX, y);
  arduboy.print(rightBuf);
}

void drawModifierInput(const char* name) {
  arduboy.setTextSize(2);
  arduboy.setTextColor(WHITE);

  drawCenteredText(name, 14, 2);

  char modBuf[5];
  formatModifier(modifier, modBuf, sizeof(modBuf));
  drawCenteredText(modBuf, 36, 2);
}

void drawTransition() {
  arduboy.setTextSize(1);
  arduboy.setTextColor(WHITE);

  char buf[16];
  strcpy(buf, "rolling");

  for (uint8_t i = 0; i < transitionDots; i++) {
    strcat(buf, ".");
  }

  const int16_t maxTextW = strlen("rolling....") * CHAR_W;
  arduboy.setCursor((SCREEN_W - maxTextW) / 2, 28);
  arduboy.print(buf);
}

void drawResults() {
  if (mode == MODE_DICE) {
    drawDiceResults();
  } else {
    drawGameResults();
  }
}

void drawDiceResults() {
  arduboy.setTextSize(1);
  arduboy.setTextColor(WHITE);

  char totalBuf[18];
  snprintf(totalBuf, sizeof(totalBuf), "TOTAL: %u", diceTotal);
  drawCenteredText(totalBuf, 8, 1);

  const uint8_t maxCols = 5;
  const uint8_t maxRows = 4;
  const uint8_t usedRows = (diceCount + maxCols - 1) / maxCols;
  const uint8_t topPadRows = (maxRows - usedRows) / 2;
  const int16_t startX = 6;
  const int16_t startY = 24;
  const uint8_t cellW = 24; // 4 chars * 6 px
  const uint8_t rowH = 10;
  uint8_t resultIndex = 0;

  for (uint8_t row = 0; row < usedRows; row++) {
    uint8_t remainingResults = diceCount - resultIndex;
    uint8_t remainingRows = usedRows - row;
    uint8_t rowCount = (remainingResults + remainingRows - 1) / remainingRows;
    uint8_t leftPadCols = (maxCols - rowCount + 1) / 2;

    for (uint8_t col = 0; col < rowCount; col++) {
      char buf[5];
      snprintf(buf, sizeof(buf), "%u", diceResults[resultIndex]);

      arduboy.setCursor(startX + (leftPadCols + col) * cellW, startY + (topPadRows + row) * rowH);
      arduboy.print(buf);

      resultIndex++;
    }
  }
}

void drawGameResults() {
  const char* resultText = getHitResultText();

  arduboy.setTextColor(WHITE);

  arduboy.setTextSize(2);
  drawCenteredText(resultText, 8, 2);

  arduboy.setTextSize(1);

  char diceBuf[28];

  if (mode == MODE_PBTA) {
    char modOp = '+';
    uint8_t modAmount = modifier;

    if (modifier < 0) {
      modOp = '-';
      modAmount = -modifier;
    }

    snprintf(
      diceBuf,
      sizeof(diceBuf),
      "[%u] + [%u] %c %u = %d",
      rollA,
      rollB,
      modOp,
      modAmount,
      rollSum
    );
  } else {
    char modOp = '+';
    uint8_t modAmount = modifier;

    if (modifier < 0) {
      modOp = '-';
      modAmount = -modifier;
    }

    snprintf(
      diceBuf,
      sizeof(diceBuf),
      "[%u] %c %u = %d VS %u, %u",
      rollA,
      modOp,
      modAmount,
      rollSum,
      rollB,
      rollC
    );
  }

  drawCenteredText(diceBuf, 34, 1);

  if (match) {
    drawCenteredText("MATCH", 48, 1);
  }
}

const char* getHitResultText() {
  switch (hitResult) {
    case HIT_MISS:
      return "Miss";

    case HIT_WEAK:
      return "Weak Hit";

    case HIT_STRONG:
      return "Strong Hit";
  }

  return "";
}

void formatModifier(int8_t mod, char* buf, size_t bufSize) {
  if (mod >= 0) {
    snprintf(buf, bufSize, "+%d", mod);
  } else {
    snprintf(buf, bufSize, "%d", mod);
  }
}

void drawCenteredText(const char* text, int16_t y, uint8_t textSize) {
  int16_t textW = strlen(text) * CHAR_W * textSize;
  int16_t x = (SCREEN_W - textW) / 2;

  if (x < 0) {
    x = 0;
  }

  arduboy.setCursor(x, y);
  arduboy.print(text);
}
