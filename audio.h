struct Note {
  int freq;
  int duration; // ms
};

const Note marioSound[] = {
  {659, 115},
  {784, 115},
  {1319, 115},
  {1047, 115},
  {1175, 115},
  {1568, 115}
};

const int numNotes = sizeof(marioSound) / sizeof(marioSound[0]);

unsigned long lastTime = 0;
int currentNote = 0;
bool playing = false;

void speakerTone() {
  currentNote = 0;
  lastTime = millis();
  playing = true;
  tone(speaker, marioSound[currentNote].freq, marioSound[currentNote].duration);
}

void updateAudio() {
  if(!playing) return;

  unsigned long now = millis();
  if(now - lastTime >= marioSound[currentNote].duration){
    currentNote++;
    if(currentNote >= numNotes){
      noTone(speaker);
      playing = false;
    } else {
      tone(speaker, marioSound[currentNote].freq, marioSound[currentNote].duration);
      lastTime = now;
    }
  }
}
