struct Note {
  int freq;
  int duration; // ms
};

const Note marioNotes[] = {
  {659, 115},
  {784, 115},
  {1319, 115},
  {1047, 115},
  {1175, 115},
  {1568, 115}
};

const int numNotes = sizeof(marioNotes) / sizeof(marioNotes[0]);

unsigned long lastTime = 0;
int currentNote = 0;
bool playing = false;

void marioSound() {
  currentNote = 0;
  lastTime = millis();
  playing = true;
  tone(speaker, marioNotes[currentNote].freq, marioNotes[currentNote].duration);
}

void updateMarioSound() {
  if(!playing) return;

  unsigned long now = millis();
  if(now - lastTime >= marioNotes[currentNote].duration){
    currentNote++;
    if(currentNote >= numNotes){
      noTone(speaker);
      playing = false;
    } else {
      tone(speaker, marioNotes[currentNote].freq, marioNotes[currentNote].duration);
      lastTime = now;
    }
  }
}
