export default function Block(props) {
    const {
        isGameOver,
        isOpen,
        openNumber,
        isFlagged,
        hasMine,
        probability,
        isBest,
        isPreferred,
        onProbe,
    } = props;

    let tdClass = 'blk';
    let spanClass = '';
    let text = '';

    if (isOpen) {
        tdClass += ' blk-open';
        if (hasMine) {
            tdClass += ' blk-dead';
            text = 'M';
        } else if (openNumber) {
            spanClass = `num-${openNumber}`;
            text = openNumber;
        }
    } else if (!isGameOver) {
        if (isFlagged) {
            spanClass = 'flag';
            text = 'F';
        }
        if (probability === 0)
            tdClass += ' blk-safe';
        else if (probability === 1)
            tdClass += ' blk-dangerous';
        if (isBest)
            tdClass += ' blk-best';
        if (isPreferred)
            tdClass += ' blk-preferred';
    } else {
        if (isFlagged && !hasMine) {
            spanClass = 'flag bad-flag';
            text = 'F';
        } else if (isFlagged) {
            spanClass = 'flag';
            text = 'F';
        } else if (hasMine) {
            text = 'M';
        }
    }

    return (<td className={tdClass}>
        <span className={spanClass}>{text}</span>
    </td>);
}
