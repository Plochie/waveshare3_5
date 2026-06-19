import { DeckPage } from "../types";

interface Props {
  page: DeckPage;
  cols: number;
  rows: number;
  selectedPos: number | null;
  onSelect: (pos: number) => void;
}

export default function GridView({ page, cols, rows, selectedPos, onSelect }: Props) {
  const byPos = new Map(page.buttons.map((b) => [b.pos, b]));
  const slots = Array.from({ length: cols * rows }, (_, pos) => pos);

  return (
    <div className="grid-view" style={{ gridTemplateColumns: `repeat(${cols}, 1fr)` }}>
      {slots.map((pos) => {
        const btn = byPos.get(pos);
        const selected = pos === selectedPos;
        return (
          <button
            type="button"
            key={pos}
            className={`tile ${btn ? "filled" : "empty"} ${selected ? "selected" : ""}`}
            style={btn?.color ? { background: btn.color } : undefined}
            onClick={() => onSelect(pos)}
          >
            {btn ? (
              <>
                <span className="tile-label">{btn.label || "(no label)"}</span>
                {btn.open_page && <span className="tile-badge">→ {btn.open_page}</span>}
              </>
            ) : (
              <span className="tile-add">+</span>
            )}
          </button>
        );
      })}
    </div>
  );
}
