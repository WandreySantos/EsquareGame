import { StyleSheet } from 'react-native';

export default StyleSheet.create({
  container: { flex: 1, justifyContent: 'center', alignItems: 'center' },
  title: { fontSize: 24, fontWeight: 'bold', marginBottom: 20 },
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
    backgroundColor: '#000',
    padding: 10,
    borderRadius: 5,
  },
  resetText: { color: '#fff', fontSize: 18 },
  menuButton: {
    backgroundColor: '#222',
    paddingVertical: 15,
    paddingHorizontal: 40,
    borderRadius: 10,
    marginVertical: 10,
  },
  menuText: { color: '#fff', fontSize: 20 },
  turnText: { fontSize: 18, marginBottom: 20 },
  directionButton: {
    backgroundColor: '#222',
    padding: 20,
    margin: 5,
    borderRadius: 10,
    justifyContent: 'center',
    alignItems: 'center',
  },
});
