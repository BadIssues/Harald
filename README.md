# Harald - Gestionnaire de Sessions Xbox Cloud Gaming

Harald est un gestionnaire de sessions pour Xbox Cloud Gaming sur Windows, permettant de gérer jusqu'à 12 sessions simultanées avec une interface native Win32.

## Fonctionnalités

- Gestion de jusqu'à 12 sessions simultanées
- Interface utilisateur native Win32 responsive
- Système anti-AFK intégré
- Synchronisation des entrées entre les sessions
- Gestion des profils utilisateurs
- Intégration WebView2 pour l'affichage des sessions
- Interface responsive adaptative

## Prérequis

- Windows 10 ou supérieur
- Microsoft Edge WebView2 Runtime
- Compilateur GCC pour Windows
- Bibliothèques de développement Windows (GDI32, User32)

## Installation

1. Installez le Microsoft Edge WebView2 Runtime depuis [le site officiel de Microsoft](https://developer.microsoft.com/en-us/microsoft-edge/webview2/)
2. Clonez ce dépôt
3. Assurez-vous que les fichiers WebView2 sont présents dans le dossier `lib`
4. Compilez le projet avec la commande :
```bash
gcc -w -o build/harald.exe src/*.c -Iinclude -Ilib -Llib -lgdi32 -luser32 -lwebview2loader
```

## Configuration

Le fichier `config/config.ini` permet de configurer :
- Nombre maximum de sessions
- Intervalle anti-AFK
- Interface réseau
- Activation de l'interface utilisateur
- URL du jeu par défaut

## Utilisation

1. Lancez l'application
2. Utilisez l'interface pour gérer vos sessions
3. Chaque session affiche le jeu dans une WebView intégrée
4. Les contrôles sont synchronisés entre les sessions actives

## Structure du Projet

```
harald/
├── build/              # Fichiers de compilation
├── config/            # Fichiers de configuration
├── include/           # Fichiers d'en-tête
├── lib/              # Bibliothèques externes
│   ├── WebView2.h    # En-tête WebView2
│   └── webview2loader.dll # Bibliothèque WebView2
├── src/              # Code source
└── README.md         # Documentation
```

## Dépendances

- Microsoft Edge WebView2 Runtime
- Bibliothèques Windows (GDI32, User32)
- WebView2Loader.dll

## Licence

Ce projet est sous licence MIT. Voir le fichier LICENSE pour plus de détails.

## 🤝 Contribution

Les contributions sont les bienvenues ! N'hésitez pas à :
1. Fork le projet
2. Créer une branche pour votre fonctionnalité
3. Commiter vos changements
4. Pousser vers la branche
5. Ouvrir une Pull Request

## 📝 Licence

Ce projet est sous licence MIT. Voir le fichier `LICENSE` pour plus de détails.

## ⚠️ Avertissement

Ce projet est fourni tel quel, sans garantie d'aucune sorte. L'utilisation est à vos propres risques et doit respecter les conditions d'utilisation de Xbox Cloud Gaming.

## 🛠️ Développement

### Structure du Projet
```
harald/
├── src/           # Code source
├── include/       # Headers
├── resources/     # Ressources
├── config/        # Configuration
└── build/         # Fichiers de build
```

