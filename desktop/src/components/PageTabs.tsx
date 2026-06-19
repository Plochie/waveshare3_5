import { DeckPage } from "../types";

interface Props {
  pages: DeckPage[];
  currentPageId: string;
  onSelect: (id: string) => void;
  onAdd: () => void;
  onRename: (id: string, title: string) => void;
  onRemove: (id: string) => void;
}

export default function PageTabs({ pages, currentPageId, onSelect, onAdd, onRename, onRemove }: Props) {
  return (
    <div className="page-tabs">
      {pages.map((p) => (
        <div key={p.id} className={`page-tab ${p.id === currentPageId ? "active" : ""}`}>
          <button type="button" className="page-tab-select" onClick={() => onSelect(p.id)}>
            <input
              className="page-title-input"
              value={p.title}
              onChange={(e) => onRename(p.id, e.target.value)}
              onClick={(e) => e.stopPropagation()}
            />
            <span className="page-id">{p.id}</span>
          </button>
          {p.id !== "home" && (
            <button type="button" className="icon-btn" onClick={() => onRemove(p.id)}>
              ✕
            </button>
          )}
        </div>
      ))}
      <button type="button" className="link-btn" onClick={onAdd}>
        + Page
      </button>
    </div>
  );
}
