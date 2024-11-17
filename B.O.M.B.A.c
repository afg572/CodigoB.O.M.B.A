const int led1 = 2;
const int led2 = 3;
const int led3 = 4;
const int button1 = 5;
const int button2 = 6;
const int button3 = 7;
const int buzzer = 8;

// Variáveis de estado
int desarmeSequencia[3]; // Sequência correta
int desarmeIndex = 0;    // Índice atual da sequência
int tentativas = 0;      // Contador de tentativas

// Tempo da contagem regressiva
int tempoRestante = 30; // 30 segundos
unsigned long ultimoTempo = 0;

// Variáveis de debounce
const int debounceDelay = 200; // Tempo de debounce em milissegundos
unsigned long ultimaLeituraButton1 = 0;
unsigned long ultimaLeituraButton2 = 0;
unsigned long ultimaLeituraButton3 = 0;

void setup() {
  // Configurar LEDs e botões como entrada e saída
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  // Iniciar comunicação serial para depuração
  Serial.begin(9600);
  Serial.println("Iniciando sistema de desarme...");
  
  // Inicializar o gerador de números aleatórios
  randomSeed(analogRead(0));
  
  gerarSequenciaAleatoria(); // Gera a sequência inicial
  reiniciarSistema();        // Inicia o sistema
}

void loop() {
  // Verificar o tempo restante
  if (millis() - ultimoTempo >= 1000) {
    tempoRestante--;
    ultimoTempo = millis();
    Serial.print("Tempo restante: ");
    Serial.println(tempoRestante);

    if (tempoRestante == 0) {
      explodirBomba();
    }
  }

  // Verificar se o botão atual da sequência foi pressionado com debounce
  if (botaoPressionadoComDebounce(desarmeSequencia[desarmeIndex])) {
    Serial.print("Botão ");
    Serial.print(desarmeSequencia[desarmeIndex]);
    Serial.println(" pressionado corretamente.");

    desarmeIndex++;
    if (desarmeIndex == 3) {
      desarmarBomba();
    }
  } 
  // Se um botão fora da sequência for pressionado, contar como uma tentativa errada
  else if ((botaoPressionadoComDebounce(button1) && desarmeSequencia[desarmeIndex] != button1) ||
           (botaoPressionadoComDebounce(button2) && desarmeSequencia[desarmeIndex] != button2) ||
           (botaoPressionadoComDebounce(button3) && desarmeSequencia[desarmeIndex] != button3)) {
    tentativas++;
    Serial.print("Tentativa errada! Tentativas restantes: ");
    Serial.println(3 - tentativas);
    emitirSomErro();        // Emitir som curto de erro
    apagarLed(tentativas);  // Apaga um LED com base no número de tentativas

    if (tentativas >= 3) {
      Serial.println("Tentativas esgotadas! Explodindo bomba com som prolongado.");
      explodirBombaFinal();
    } else {
      reiniciarSistema(); // Reinicia o sistema após uma tentativa errada, sem gerar nova sequência
    }
  }
}

void desarmarBomba() {
  Serial.println("Bomba desarmada!");
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  tone(8,3620,1000);
  delay(3000); // Pausa antes de reiniciar
  gerarSequenciaAleatoria(); // Gera nova sequência após desarmar
  reiniciarSistema();
}

void explodirBomba() {
  Serial.println("Bomba explodiu!");
  tone(buzzer, 1000); // Buzzer para o som de explosão
  delay(2000);
  noTone(buzzer);
  delay(1000); // Pausa antes de reiniciar
  gerarSequenciaAleatoria(); // Gera nova sequência após explodir
  reiniciarSistema();
}

void explodirBombaFinal() {
  Serial.println("Bomba explodiu com som prolongado!");
  tone(buzzer,3000,1000); // Buzzer para o som de explosão prolongado
  delay(2000);        // Som de explosão mais longo
 
  while (true);       // Parar o código, sistema se fecha
}

void reiniciarSistema() {
  Serial.println("Reiniciando sistema...");
  desarmeIndex = 0;         // Reinicia a sequência de desarmamento
  tempoRestante = 30;       // Reinicia o tempo da contagem regressiva
  ultimoTempo = millis();   // Reinicia o tempo de referência
  
  // Reacende os LEDs restantes com base nas tentativas
  if (tentativas < 3) digitalWrite(led1, HIGH);
  if (tentativas < 2) digitalWrite(led2, HIGH);
  if (tentativas < 1) digitalWrite(led3, HIGH);
}

void apagarLed(int tentativa) {
  // Apaga um LED com base na tentativa errada
  switch(tentativa) {
    case 1:
      digitalWrite(led3, LOW); // Apaga o terceiro LED na primeira tentativa errada
      break;
    case 2:
      digitalWrite(led2, LOW); // Apaga o segundo LED na segunda tentativa errada
      break;
    case 3:
      digitalWrite(led1, LOW); // Apaga o primeiro LED na terceira tentativa errada
      break;
  }
}

void emitirSomErro() {
  tone(buzzer, 1000); // Som de erro curto
  delay(200);         // Duração do som de erro
  noTone(buzzer);
}

bool botaoPressionadoComDebounce(int botao) {
  unsigned long tempoAtual = millis();
  
  // Verificar qual botão foi pressionado e aplicar debounce
  if (botao == button1) {
    if (digitalRead(button1) == LOW && (tempoAtual - ultimaLeituraButton1) > debounceDelay) {
      ultimaLeituraButton1 = tempoAtual;
      return true;
    }
  } else if (botao == button2) {
    if (digitalRead(button2) == LOW && (tempoAtual - ultimaLeituraButton2) > debounceDelay) {
      ultimaLeituraButton2 = tempoAtual;
      return true;
    }
  } else if (botao == button3) {
    if (digitalRead(button3) == LOW && (tempoAtual - ultimaLeituraButton3) > debounceDelay) {
      ultimaLeituraButton3 = tempoAtual;
      return true;
    }
  }
  return false;
}

void gerarSequenciaAleatoria() {
  int botoes[] = { button1, button2, button3 };

  // Embaralha a sequência
  for (int i = 0; i < 3; i++) {
    int indiceAleatorio = random(i, 3);
    int temp = botoes[i];
    botoes[i] = botoes[indiceAleatorio];
    botoes[indiceAleatorio] = temp;
  }

  // Copiar a sequência aleatória para desarmeSequencia
  for (int i = 0; i < 3; i++) {
    desarmeSequencia[i] = botoes[i];
    Serial.print("Posição ");
    Serial.print(i + 1);
    Serial.print(": Botão no pino ");
    Serial.println(desarmeSequencia[i]);
  }
}