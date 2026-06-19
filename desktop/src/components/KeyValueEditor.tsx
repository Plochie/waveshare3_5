interface Props {
  value: Record<string, string>;
  onChange: (next: Record<string, string>) => void;
  keyPlaceholder?: string;
  valuePlaceholder?: string;
}

export default function KeyValueEditor({
  value,
  onChange,
  keyPlaceholder = "key",
  valuePlaceholder = "value",
}: Props) {
  const entries = Object.entries(value);

  function updateEntry(index: number, key: string, val: string) {
    const next = entries.map((e, i) => (i === index ? [key, val] : e));
    onChange(Object.fromEntries(next));
  }

  function removeEntry(index: number) {
    const next = entries.filter((_, i) => i !== index);
    onChange(Object.fromEntries(next));
  }

  function addEntry() {
    onChange({ ...value, "": "" });
  }

  return (
    <div className="kv-editor">
      {entries.map(([k, v], i) => (
        <div className="kv-row" key={i}>
          <input
            placeholder={keyPlaceholder}
            value={k}
            onChange={(e) => updateEntry(i, e.target.value, v)}
          />
          <input
            placeholder={valuePlaceholder}
            value={v}
            onChange={(e) => updateEntry(i, k, e.target.value)}
          />
          <button type="button" className="icon-btn" onClick={() => removeEntry(i)}>
            ✕
          </button>
        </div>
      ))}
      <button type="button" className="link-btn" onClick={addEntry}>
        + Add row
      </button>
    </div>
  );
}
