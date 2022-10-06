import './App.css';
import MineSweeperSolver from './MineSweeperSolver';
import Board from './Board';

MineSweeperSolver({ locateFile: () => 'MineSweeperSolver.wasm' }).then(async (Module) => {
  console.log(Module);
});

function App() {
  return (
    <div className="App">
        <Board
            width={30}
            height={16}
        ></Board>
    </div>
  );
}

export default App;
