// kmp_fichier.cpp
// Compilation : g++ -std=c++17 -O2 kmp_fichier.cpp -o kmp_fichier

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

long long nbComparaisons = 0;

string enMinuscules(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return tolower(c); });
    return s;
}

// Tableau LPS : plus long prefixe qui est aussi suffixe
vector<int> calculerLPS(const string& motif) {
    int m = motif.size();
    vector<int> lps(m, 0);
    int len = 0;

    for (int i = 1; i < m; ) {
        if (motif[i] == motif[len]) {
            lps[i++] = ++len;
        } else if (len > 0) {
            len = lps[len - 1];
        } else {
            lps[i++] = 0;
        }
    }
    return lps;
}

// Toutes les positions du motif dans le texte (le LPS est calcule une seule fois)
vector<int> KMP(const string& texte, const string& motif, const vector<int>& lps) {
    vector<int> positions;
    int n = texte.size();
    int m = motif.size();
    if (m > n) return positions;

    int j = 0;
    for (int i = 0; i < n; ) {          // i ne recule jamais
        ++nbComparaisons;
        if (texte[i] == motif[j]) {
            i++;
            j++;
            if (j == m) {
                positions.push_back(i - m);
                j = lps[j - 1];         // occurrences qui se chevauchent
            }
        } else if (j > 0) {
            j = lps[j - 1];
        } else {
            i++;
        }
    }
    return positions;
}

// Affiche la ligne originale avec chaque occurrence entre [ ]
void afficherMarque(const string& ligne, const vector<int>& pos, int m) {
    string res;
    size_t p = 0;
    for (int i = 0; i < (int)ligne.size(); ++i) {
        if (p < pos.size() && i == pos[p]) res += '[';
        res += ligne[i];
        if (p < pos.size() && i == pos[p] + m - 1) { res += ']'; ++p; }
    }
    cout << res << "\n";
}

int main() {
    string chemin, motif, rep;

    cout << "Fichier (Entree = texte.txt) : ";
    getline(cin, chemin);
    if (chemin.empty()) chemin = "texte.txt";

    ifstream f(chemin);
    if (!f) {
        cerr << "Impossible d'ouvrir " << chemin << "\n";
        return 1;
    }

    cout << "Mot a chercher : ";
    getline(cin, motif);
    if (motif.empty()) {
        cout << "Le mot est vide.\n";
        return 1;
    }

    cout << "Ignorer la casse ? (o/n) : ";
    getline(cin, rep);
    bool ignorer = !rep.empty() && (rep[0] == 'o' || rep[0] == 'O');

    string m = ignorer ? enMinuscules(motif) : motif;
    vector<int> lps = calculerLPS(m);       // calcule UNE seule fois

    cout << "\nMotif : ";
    for (char c : m) cout << c << ' ';
    cout << "\nLPS   : ";
    for (int v : lps) cout << v << ' ';
    cout << "\n\n";

    string ligne;
    int numLigne = 0;
    long long total = 0;
    long long nbCaracteres = 0;
    int lignesTrouvees = 0;

    while (getline(f, ligne)) {
        ++numLigne;
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();
        nbCaracteres += ligne.size();

        string t = ignorer ? enMinuscules(ligne) : ligne;
        vector<int> pos = KMP(t, m, lps);

        if (!pos.empty()) {
            ++lignesTrouvees;
            total += pos.size();
            cout << "Ligne " << numLigne << ", colonne(s) ";
            for (int p : pos) cout << p + 1 << ' ';
            cout << ":\n  ";
            afficherMarque(ligne, pos, m.size());
        }
    }

    cout << "\n=== Resultat ===\n";
    if (total == 0)
        cout << "\"" << motif << "\" n'apparait pas dans " << chemin << ".\n";
    else
        cout << total << " occurrence(s) de \"" << motif << "\" dans "
             << lignesTrouvees << " ligne(s).\n";
    cout << "Lignes lues   : " << numLigne << "\n";
    cout << "Comparaisons  : " << nbComparaisons
         << " (pour " << nbCaracteres << " caracteres)\n";
    return 0;
}