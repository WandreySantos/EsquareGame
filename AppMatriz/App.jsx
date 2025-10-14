import React, { useState } from 'react';
import { View, Text, TouchableOpacity, StyleSheet } from 'react-native';

export default function App() {
  const ESP32_IP = '192.168.0.102'; // IP do ESP32
  const [board, setBoard] = useState([
    ['', '', ''],
    ['', '', ''],
    ['', '', ''],
  ]);
  const [currentPlayer, setCurrentPlayer] = useState('X');
  const [status, setStatus] = useState('Jogo em andamento');

  const play = async (x, y) => {
    if (board[x][y] !== '' || status !== 'Jogo em andamento') return;

    try {
      const res = await fetch(
        `http://${ESP32_IP}/play?x=${x}&y=${y}&player=${currentPlayer}`
      );
      const data = await res.json();

      // Atualiza tabuleiro local imediatamente
      setBoard((prev) =>
        prev.map((row, i) =>
          row.map((cell, j) => (i === x && j === y ? currentPlayer : cell))
        )
      );

      // Atualiza status do jogo
      setStatus(data.status === 'ongoing' ? 'Jogo em andamento' : data.status);

      // Alterna jogador apenas se jogo continua
      if (data.status === 'ongoing') {
        setCurrentPlayer((prev) => (prev === 'X' ? 'O' : 'X'));
      }
    } catch (e) {
      console.log('Erro:', e);
    }
  };

  const reset = async () => {
    try {
      await fetch(`http://${ESP32_IP}/reset`);
      setBoard([
        ['', '', ''],
        ['', '', ''],
        ['', '', ''],
      ]);
      setCurrentPlayer('X');
      setStatus('Jogo em andamento');
    } catch (e) {
      console.log('Erro:', e);
    }
  };

  const renderButton = (x, y) => {
    let bgColor = '#fff';
    if (board[x][y] === 'X') bgColor = '#ff4d4d';
    if (board[x][y] === 'O') bgColor = '#4d79ff';

    return (
      <TouchableOpacity
        key={`${x}-${y}`}
        style={[
          styles.cell,
          {
            backgroundColor: bgColor,
            pointerEvents:
              status !== 'Jogo em andamento' && board[x][y] === ''
                ? 'none'
                : 'auto', // só bloqueia células vazias se o jogo acabou
          },
        ]}
        onPress={() => play(x, y)}
      >
        <Text style={styles.cellText}>{board[x][y]}</Text>
      </TouchableOpacity>
    );
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Jogo da Velha - Matriz 16x16</Text>
      <Text
        style={[
          styles.turnText,
          status !== 'Jogo em andamento' && {
            color: 'green',
            fontWeight: 'bold',
          },
        ]}
      >
        {status === 'Jogo em andamento' ? `Vez de: ${currentPlayer}` : status}
      </Text>
      <View style={styles.board}>
        {[0, 1, 2].map((x) => (
          <View key={x} style={styles.row}>
            {[0, 1, 2].map((y) => renderButton(x, y))}
          </View>
        ))}
      </View>
      <TouchableOpacity style={styles.resetButton} onPress={reset}>
        <Text style={styles.resetText}>Resetar</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    padding: 20,
  },
  title: { fontSize: 24, fontWeight: 'bold', marginBottom: 10 },
  turnText: { fontSize: 18, marginBottom: 20 },
  board: {},
  row: { flexDirection: 'row' },
  cell: {
    width: 80,
    height: 80,
    borderWidth: 1,
    borderColor: '#000',
    justifyContent: 'center',
    alignItems: 'center',
  },
  cellText: { fontSize: 28, fontWeight: 'bold' },
  resetButton: {
    marginTop: 20,
    backgroundColor: '#000000ff',
    padding: 10,
    borderRadius: 5,
  },
  resetText: { color: '#fff', fontSize: 18 },
});
