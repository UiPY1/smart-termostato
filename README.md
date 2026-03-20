## Funzioni Principali

### setup()
**Descrizione:**  
Inizializza tutte le componenti hardware (WiFi, pin, LCD, sensore LM75), le proprietà cloud, e la visualizzazione su display.  
Se il sensore LM75 non è presente, mostra un errore e blocca l'esecuzione.

---

### loop()
**Descrizione:**  
Ciclo principale del programma. Gestisce:
- La lettura e la gestione dei pulsanti
- Modalità manuale/automatica
- Aggiornamento della temperatura e della data/ora
- Controllo del relè con isteresi termica
- Aggiornamento del display LCD
- Timeout della schermata di impostazione temperatura

---

### updateLCD()
**Descrizione:**  
Aggiorna il contenuto del display LCD Nokia 5110.
Mostra:
- Modalità corrente (Manuale "M" o Automatica "A")
- Pagina corrente: temperatura attuale con data/ora, oppure schermata di impostazione della temperatura target.

---

### handleButtonPress()
**Descrizione:**  
Gestisce la pressione breve del pulsante 1:
- Cambia la pagina visualizzata sul display.
- Se si è nella schermata di impostazione, incrementa il valore della temperatura target.

---

### handleButtonPressDecrement()
**Descrizione:**  
Gestisce la pressione breve del pulsante 2:
- Cambia la pagina visualizzata sul display.
- Se si è nella schermata di impostazione, decrementa il valore della temperatura target.

---

### lm75Present(uint8_t address)
**Descrizione:**  
Controlla la presenza del sensore LM75 all'indirizzo I2C specificato.
**Ritorno:** `true` se il sensore risponde, `false` altrimenti.

---

### lm75ReadTempC(uint8_t address)
**Descrizione:**  
Legge la temperatura dal sensore LM75 all’indirizzo specificato.
**Ritorno:** Valore della temperatura in gradi Celsius come float.

---

### updateDateTime()
**Descrizione:**  
Acquisisce la data e l’ora dal Cloud, converte in formato leggibile e aggiorna il buffer usato per la visualizzazione sul display LCD.

---

## Note su altre variabili & logica

- **Modalità Manuale/Automatica:**  
La modalità viene commutata tenendo premuto i pulsanti 1 o 2 per più di 3 secondi.  
- **Isteresi termica:**  
Il relè viene acceso o spento con una soglia di ±0.5°C rispetto alla temperatura target, per evitare continui cambi di stato.
- **Timeout Impostazione Temperatura:**  
Se si è nella schermata “Imposta Temperatura” per più di 2 secondi senza pressione di pulsanti, si torna alla schermata di temperatura attuale e si spegne la retroilluminazione.

---

## Utilizzo Pulsanti

- **Pulsante 1 (Incrementa):**
  - Pressione breve: cambia pagina/incrementa temperatura.
  - Pressione lunga: attiva modalità manuale.
- **Pulsante 2 (Decrementa):**
  - Pressione breve: cambia pagina/decrementa temperatura.
  - Pressione lunga: attiva modalità automatica.

---

## MenuState Enum

Permette la gestione delle due pagine visualizzabili:
- **CURRENT_TEMPERATURE_PAGE:**  
  Mostra temperatura attuale e data/ora.
- **SET_TEMPERATURE_PAGE:**  
  Permette di impostare la temperatura desiderata.

---
