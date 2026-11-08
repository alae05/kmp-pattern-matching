#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

using namespace std;

string enMinuscules(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return tolower(c); });
    return s;
}

bool oui(const string& question) {
    string s;
    cout << question << " (o/n) : ";
    getline(cin, s);
    return !s.empty() && (s[0] == 'o' || s[0] == 'O');
}

// Lettre d'un mot : alphanumerique, ou octet >= 128 (lettres accentuees en UTF-8)
bool estLettre(unsigned char c) {
    return isalnum(c) || c >= 128;
}

// Decoupe un texte en mots
vector<string> decouperEnMots(const string& ligne) {
    vector<string> mots;
    string courant;
    for (unsigned char c : ligne) {
        if (estLettre(c)) courant += (char)c;
        else if (!courant.empty()) { mots.push_back(courant); courant.clear(); }
    }
    if (!courant.empty()) mots.push_back(courant);
    return mots;
}

struct Resultat {
    size_t longueur = 0;
    size_t posA = 0;   // position dans le premier mot
    size_t posB = 0;   // position dans le second mot
};

// Plus grande sous-chaine commune entre a et b (programmation dynamique)
Resultat plusGrandeCommune(const string& a, const string& b) {
    Resultat r;
    vector<vector<size_t>> dp(a.size() + 1, vector<size_t>(b.size() + 1, 0));
    for (size_t i = 1; i <= a.size(); ++i)
        for (size_t j = 1; j <= b.size(); ++j)
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
                if (dp[i][j] > r.longueur) {
                    r.longueur = dp[i][j];
                    r.posA = i - dp[i][j];
                    r.posB = j - dp[i][j];
                }
            }
    return r;
}

// ---------- Mode 2 : comparer deux mots ----------
void modeDeuxMots(bool ignorer) {
    string a, b;
    cout << "Premier mot : ";
    getline(cin, a);
    cout << "Second mot  : ";
    getline(cin, b);
    if (a.empty() || b.empty()) { cout << "Il faut deux mots.\n"; return; }

    string ma = ignorer ? enMinuscules(a) : a;
    string mb = ignorer ? enMinuscules(b) : b;
    Resultat r = plusGrandeCommune(ma, mb);

    if (r.longueur == 0) {
        cout << "Aucun caractere commun entre \"" << a << "\" et \"" << b << "\".\n";
        return;
    }
    cout << "Plus grande sous-chaine commune : \"" << a.substr(r.posA, r.longueur)
         << "\" (" << r.longueur << " car.)\n";
    cout << "  position " << r.posA << " dans \"" << a << "\", position "
         << r.posB << " dans \"" << b << "\"\n";
}

// ---------- Mode 1 : chercher dans tout le texte ----------
bool chargerTexte(string& chemin, map<string, size_t>& mots, size_t& nbLignes, bool ignorer) {
    cout << "Fichier (Entree = sous_chaines.txt) : ";
    getline(cin, chemin);
    if (chemin.empty()) chemin = "sous_chaines.txt";

    ifstream f(chemin);
    if (!f) { cerr << "Impossible d'ouvrir " << chemin << "\n"; return false; }

    string l;
    nbLignes = 0;
    while (getline(f, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (l.empty()) continue;
        ++nbLignes;
        if (ignorer) l = enMinuscules(l);
        for (const string& w : decouperEnMots(l)) mots[w]++;
    }
    if (nbLignes == 0) { cerr << "Fichier vide.\n"; return false; }
    cout << nbLignes << " lignes lues, " << mots.size() << " mots differents.\n";
    return true;
}

void modeTexte(const map<string, size_t>& mots, bool ignorer) {
    string mot;
    cout << "Mot a chercher dans le texte : ";
    getline(cin, mot);
    if (mot.empty()) return;
    string m = ignorer ? enMinuscules(mot) : mot;

    // 1) mots du texte qui CONTIENNENT le mot saisi (chat -> chat, chaton, achat, chateau)
    vector<pair<string, size_t>> contenant;
    for (const auto& kv : mots)
        if (kv.first.find(m) != string::npos) contenant.push_back(kv);

    if (!contenant.empty()) {
        sort(contenant.begin(), contenant.end(), [](const auto& x, const auto& y) {
            if (x.first.size() != y.first.size()) return x.first.size() > y.first.size();
            return x.first < y.first;
        });

        size_t total = 0;
        for (const auto& c : contenant) total += c.second;
        cout << "\"" << mot << "\" est contenu dans " << contenant.size()
             << " mot(s) different(s) du texte (" << total << " occurrences au total) :\n";

        size_t aff = min<size_t>(contenant.size(), 15);
        for (size_t i = 0; i < aff; ++i)
            cout << "  \"" << contenant[i].first << "\" (" << contenant[i].first.size()
                 << " car.) x" << contenant[i].second << "\n";
        if (contenant.size() > aff) cout << "  ... et " << contenant.size() - aff << " autre(s)\n";

        size_t maxLen = contenant[0].first.size();
        cout << "Le(s) plus long(s) (" << maxLen << " car.) :";
        for (const auto& c : contenant)
            if (c.first.size() == maxLen) cout << " \"" << c.first << "\"";
        cout << "\n";
        return;
    }

    // 2) aucun mot ne contient le mot entier : on cherche la meilleure ressemblance partielle
    cout << "Aucun mot du texte ne contient \"" << mot << "\" en entier.\n";
    size_t meilleur = 0;
    vector<pair<string, string>> best;   // (mot du texte, partie commune)
    for (const auto& kv : mots) {
        Resultat r = plusGrandeCommune(m, kv.first);
        if (r.longueur == 0 || r.longueur < meilleur) continue;
        if (r.longueur > meilleur) { meilleur = r.longueur; best.clear(); }
        best.push_back({kv.first, m.substr(r.posA, r.longueur)});
    }
    if (meilleur == 0) { cout << "Aucun caractere commun avec le texte.\n"; return; }

    cout << "Meilleure partie commune : " << meilleur << " car. dans " << best.size() << " mot(s) :\n";
    size_t aff = min<size_t>(best.size(), 15);
    for (size_t i = 0; i < aff; ++i)
        cout << "  \"" << best[i].second << "\" dans \"" << best[i].first << "\"\n";
    if (best.size() > aff) cout << "  ... et " << best.size() - aff << " autre(s)\n";
}

int main() {
    bool ignorer = oui("Ignorer la casse ?");

    map<string, size_t> mots;
    string chemin;
    size_t nbLignes = 0;
    bool charge = false;

    while (true) {
        cout << "\n=== MENU ===\n"
             << "1. Chercher un mot dans tout le texte (fichier)\n"
             << "2. Comparer deux mots entre eux (hors texte)\n"
             << "0. Quitter\n"
             << "Choix : ";
        string choix;
        if (!getline(cin, choix) || choix.empty() || choix == "0") break;

        if (choix == "1") {
            if (!charge) {
                charge = chargerTexte(chemin, mots, nbLignes, ignorer);
                if (!charge) continue;
            }
            modeTexte(mots, ignorer);
        } else if (choix == "2") {
            modeDeuxMots(ignorer);
        } else {
            cout << "Choix invalide.\n";
        }
    }
    return 0;
}