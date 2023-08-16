#include <MIDIUSB.h>
#include <Bounce2.h>

int pinosAnalogicos[3] = {A0, A1, A2};

int pinosBotoes[6] = {2, 3, 4, 5, 6, 7};
int pinosLeds[6] = {8, 9, 10, 11, 12, 13};

int notasMIDI[6] = {40, 41, 42, 36, 37, 38}; //Padrão para Ableton
int ccMIDI[3] = {16, 17, 18};
int MidiChan = 3;

int leituraAnalogicaAtual[3] = {0, 0, 0};
int ultimasLeiturasAnalogicas[3] = {0, 0, 0};
float leituraAnalogicaFiltrada[3] = {0.0, 0.0, 0.0};

Bounce botao1 = Bounce();
Bounce botao2 = Bounce();
Bounce botao3 = Bounce();
Bounce botao4 = Bounce();
Bounce botao5 = Bounce();
Bounce botao6 = Bounce();

Bounce* botoes[6] = {&botao1, &botao2, &botao3, &botao4, &botao5, &botao6};

void setup() {
  for (int i = 0; i < 6; i++) {
    pinMode(pinosBotoes[i], INPUT);
    botoes[i]->attach(pinosBotoes[i]);
    botoes[i]->interval(10);
    pinMode(pinosLeds[i], OUTPUT);
  }
  Serial.begin(115200);
  Serial.println("Iniciando comunicacao serial");
}

void loop() {
  for (int i = 0; i < 3; i++) {
    leituraAnalogicaAtual[i] = analogRead(pinosAnalogicos[i]);
    leituraAnalogicaFiltrada[i] = filtrar(leituraAnalogicaAtual[i], leituraAnalogicaFiltrada[i], 0.9);
    int leituraAnalogicaMapeada = map(leituraAnalogicaFiltrada[i], 0, 1023, 0, 127);
    if (leituraAnalogicaMapeada != ultimasLeiturasAnalogicas[i]) {
      controlChange(MidiChan - 1, ccMIDI[i], leituraAnalogicaMapeada);
    }
    MidiUSB.flush();
    ultimasLeiturasAnalogicas[i] = leituraAnalogicaMapeada;
  }
  for (int i = 0; i < 6; i++) {
    botoes[i]->update();
    int index = i + 1;
    if (botoes[i]->rose()) {
      Serial.print("Apertou: botão ");
      Serial.println(i);
      digitalWrite(pinosLeds[i], HIGH);
      noteOn(MidiChan - 1, notasMIDI[i], 127);   // Channel 3, middle C, normal velocity
      MidiUSB.flush();
    }
    if (botoes[i]->fell()) {
      Serial.print("Levantou: botão ");
      Serial.println(i);
      digitalWrite(pinosLeds[i], LOW);
      noteOff(MidiChan - 1, notasMIDI[i], 0);   // Channel 3, middle C, normal velocity
      MidiUSB.flush();
    }
  }
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

int filtrar(int valEntrada, int valFiltrado, float qtdeDeFiltragem) {
  return ((1 - qtdeDeFiltragem) * valEntrada) + (qtdeDeFiltragem * valFiltrado);
}
