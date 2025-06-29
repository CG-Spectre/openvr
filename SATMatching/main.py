import io, random, requests
import re
from PyPDF2 import PdfReader

PDF_URL = "https://img.sparknotes.com/content/testprep/pdf/sat.vocab.pdf"
HEADERS = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                  "(KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36"
}

def fetch_vocab(pdf_url=PDF_URL):
    resp = requests.get(pdf_url, headers=HEADERS)
    resp.raise_for_status()
    pdf = PdfReader(io.BytesIO(resp.content))
    text = ""
    for page in pdf.pages:
        text += page.extract_text() + "\n"
        print(page.extract_text())
    # pattern: word (pos.) definition
    entries = re.findall(r"^([a-zA-Z\-]+)\s*\((adj\.|n\.|v\.|adv\.|int\.)\)\s*(.+)", text, flags=re.MULTILINE)
    vocab = {}
    for word, pos, definition in entries:
        full_def = f"({pos}) {definition.strip()}"
        vocab[word.lower()] = full_def
        print(word.lower())
    return vocab

def play_round(vocab):
    word, correct_def = random.choice(list(vocab.items()))
    wrong_defs = random.sample([d for w, d in vocab.items() if w != word], 3)
    options = wrong_defs + [correct_def]
    random.shuffle(options)
    print(f"\nWhat is the definition of **{word}**?\n")
    for i, opt in enumerate(options, 1):
        print(f"{i}. {opt}")
    ans = input("Your choice (1–4): ").strip()
    if ans.isdigit() and options[int(ans) - 1] == correct_def:
        print("✅ Correct!")
        return True
    else:
        print(f"❌ Incorrect! The correct answer was: {correct_def}")
        return False

def main():
    print("📥 Downloading SAT vocab...")
    vocab = fetch_vocab()
    print(f"✅ Loaded {len(vocab)} vocab words!\n")
    rounds = input("How many rounds would you like to play? ").strip()
    rounds = int(rounds) if rounds.isdigit() else 10
    score = 0
    for i in range(rounds):
        print(f"\n🎲 Round {i+1} of {rounds}")
        if play_round(vocab):
            score += 1
    print(f"\n🏁 Finished! Your score: {score}/{rounds} ({(score/rounds)*100:.1f}%)")

if __name__ == "__main__":
    main()