import './App.css';
import MineSweeperSolver from './MineSweeperSolver';

MineSweeperSolver({ locateFile: () => 'MineSweeperSolver.wasm' }).then(async (Module) => {
  console.log(Module);
});

function App() {
  return (
    <div className="App">
      <header className="App-header">
        <p>
          Edit <code>src/App.js</code> and save to reload.
        </p>
      </header>
    </div>
  );
}

export default App;
