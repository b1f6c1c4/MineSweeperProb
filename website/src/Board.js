import Block from "./Block";

export default function Board(props) {
    const {
        width,
        height,
    } = props;

    if (!(width && height)) {
        return (<table className="board"></table>);
    }

    const body = [];
    for (let i = 0; i < height; i++) {
        const row = [];
        for (let j = 0; j < width; j++) {
            row.push(<Block key={j}
                            isGameOver
                            openNumber={j ? j : null} />);
        }
        body.push(<tr key={i}>{row}</tr>);
    }
    return (<table className="board">
        <tbody>
            {body}
        </tbody>
    </table>);
}