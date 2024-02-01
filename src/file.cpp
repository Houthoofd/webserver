#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <winsock2.h>

using namespace std;

std::string GetFileNameFromPath(const std::string& filePath) {
    size_t lastSeparatorPos = filePath.find_last_of("/\\");
    if (lastSeparatorPos != std::string::npos) {
        return filePath.substr(lastSeparatorPos + 1);
    }
    return filePath;
}

bool estPremier(int nombre) {
    if (nombre <= 1) {
        return false; // Les nombres inférieurs ou égaux à 1 ne sont pas premiers
    }

    for (int diviseur = 2; diviseur * diviseur <= nombre; diviseur++) {
        if (nombre % diviseur == 0) {
            std::cout << nombre << "\t n'est pas premier" << std::endl;
            return false; // Le nombre est divisible par diviseur, donc il n'est pas premier
        }
    }
    std::cout << nombre << "\t premier" << std::endl;
    return true; // Le nombre est premier
}

void EnregistrerDansFichiers(int length) {
    std::ofstream fichierHTML("./interface/resultats.html");
    std::ofstream fichierJS("./interface/graphique.js");
    std::ofstream fichierJS2("./interface/server.js");
    std::ofstream fichierCSS("./interface/styles.css");

    if (!fichierHTML.is_open() || !fichierJS.is_open() || !fichierCSS.is_open()) {
        std::cerr << "Erreur lors de l'ouverture des fichiers." << std::endl;
        return;
    }

    int rowCount = length / 10 + (length % 10 != 0); // Nombre de lignes nécessaires pour afficher tous les nombres (en excluant le zéro)
    int currentNumber = 1; // On commence à partir de 1 au lieu de zéro

    // Écriture du contenu HTML dans le fichier 'resultats.html'
    fichierHTML << "<html><head><title>Résultats des nombres premiers</title>";
    fichierHTML << "<link rel='stylesheet' type='text/css' href='styles.css'>";
    fichierHTML << "</head><body>";
    fichierHTML << "<div class='datas'><table id=tableauResultats>";

    // Déclarez un vecteur pour stocker les nombres premiers
    std::vector<int> nombresPremiers;

    for (int row = 0; row < rowCount; row++) {
        fichierHTML << "<tr>";
        for (int col = 0; col < 10; col++) {
            if (currentNumber > length)
                break;

                bool est_nombre_premier = estPremier(currentNumber);
            if (est_nombre_premier) {
                fichierHTML << "<td class='premier'>" << currentNumber << "</td>";

                // Ajoutez le nombre premier au vecteur
                nombresPremiers.push_back(currentNumber);
            } else {
                fichierHTML << "<td class='pas_premier'>" << currentNumber << "</td>";
        }
        currentNumber++;
    }
    fichierHTML << "</tr>";
}

    fichierHTML << "</table>";

    // Ajout du bouton pour générer le graphique
    fichierHTML << "<canvas id='graphique'></canvas></div>";
    fichierHTML << "<div class='panel'><input type='text' placeholder='n' name='n'><button id='envoyer'>Graphique</button>";
    fichierHTML << "<button id='genererGraphique'>Graphique</button></div><div class='statistics'>"<< nombresPremiers.size() <<"</div>";

    fichierHTML << "<script src='graphique.js'></script>";
    fichierHTML << "<script src='server.js'></script>";
    fichierHTML << "</body></html>";
    fichierHTML.close();

    // Écriture du code JavaScript dans le fichier 'graphique.js'
    std::string codeJS = R"(
        document.addEventListener('DOMContentLoaded', function() {
            var bouton = document.getElementById('genererGraphique');
            var envoyer = document.getElementById('envoyer');
            bouton.addEventListener('click', function() {
                console.log('essai');
                genererGraphique();
            });

            function genererGraphique() {
                var canvas = document.getElementById('graphique');
                console.log(canvas);
                if (!canvas) {
                    console.error("Le canvas graphique n'a pas été trouvé.");
                    return;
                }
            
                var context = canvas.getContext('2d');
            
                if (!context) {
                    console.error("Impossible d'obtenir le contexte 2D du canvas.");
                    return;
                }
            
                var donnees = [];
                var tableau = document.getElementById('tableauResultats');
                
                if (!tableau) {
                    console.error("Le tableau des résultats n'a pas été trouvé.");
                    return;
                }
            
                var lignes = tableau.getElementsByTagName('tr');
                console.log(lignes.length);
            
                for (var i = 0; i < lignes.length; i++) {
                    var colonnes = lignes[i].querySelectorAll('td');
                    for(var j = 0; j < colonnes.length; j++){
                        var nombre = parseInt(colonnes[j].innerText);
                        if(colonnes[j].classList.value=='premier'){
                            donnees.push({ nombre: nombre, estPremier: true });
                        }else{
                            donnees.push({ nombre: nombre, estPremier: false });
                        }
                    }
                }
            
                var largeur = canvas.width;
                var hauteur = canvas.height;
                var hauteurMax = Math.max(...donnees.map(d => d.estPremier));
            
                context.clearRect(0, 0, largeur, hauteur);
            
                // Dessiner les points
                console.log(donnees);
                for (var i = 0; i < donnees.length; i++) {
                    var donnee = donnees[i];
            
                    var x = i * (largeur / donnees.length);
                    var y = hauteur - (donnee.estPremier * hauteur / hauteurMax);
            
                    // Dessiner le point
                    context.fillStyle = donnee.estPremier ? 'green' : 'red';
                    context.beginPath();
                    context.arc(x, y, 4, 0, Math.PI * 2);
                    context.fill();
            
                    // Dessiner le texte du nombre
                    context.fillStyle = 'black';
                    context.font = '12px Arial';
                    context.textAlign = 'center';
                    context.fillText(donnee.nombre.toString(), x, y - 10);
                }
            }
        });
    )";

    fichierJS << codeJS;
    fichierJS.close();

    std::string codeJS2 = R"(
            envoyer.addEventListener('click', function() {
                let value = document.querySelector('input').value;
                let jsonData = JSON.stringify({ value: value });

                try {
                    fetch('http://localhost:8080/interface/resultats.html', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: jsonData
                })
            .then(response => {
                if (response.ok) {
                    console.log('Données envoyées avec succès au serveur');
                } else {
                    console.log('Erreur lors de l\'envoi des données au serveur');
                }
            })
            .catch(error => {
                console.error('Une erreur s\'est produite lors de l\'envoi des données au serveur:', error);
            });
        } catch (error) {
            console.error('Une erreur s\'est produite lors de l\'envoi des données au serveur:', error);
        }
    });

    )";

    fichierJS2 << codeJS2;
    fichierJS2.close();

    // Écriture du code CSS dans le fichier 'styles.css'
    std::string codeCSS = "body{display: flex;flex-direction: column;}\ntd.premier{text-align: center;background-color:green;}\ntd.pas_premier{text-align:center;background-color:red;}.datas {display: flex;flex-direction: inherit;}\n";
    fichierCSS << codeCSS;
    fichierCSS.close();

    std::cout << "Les résultats ont été enregistrés dans les fichiers 'resultats.html', 'graphique.js' et 'styles.css'." << std::endl;
}