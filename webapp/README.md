# Frontend pro napájecí jednotku

## Prerekvizity

- Node.js 18.x
- NPM

## Instalace

1. Nainstalujte závislosti pomocí příkazu:
	 ```bash
	 npm install
	 ```
2. Vytvořte soubor `.env.local` s konfigurací:
	```dotenv
	# REST API base URL
	VITE_API_BASE_URL=/api/v1    # URL backendu (např. /api/v1)
	# Localization
	VITE_I18N_LOCALE=en          # Výchozí lokalizace (musí být v seznamu podporovaných lokalizací)
	VITE_I18N_FALLBACK_LOCALE=en # Výchozí lokalizace pro případ, že nějaký text není přeložen
	# Sentry error tracking
	VITE_SENTRY_ENABLED=false    # Zapnutí hlášení chyb do Sentry
	VITE_SENTRY_DSN=             # DSN pro Sentry
	VITE_SENTRY_URL=             # URL Sentry serveru
	VITE_SENTRY_ORG=             # Název organizace v Sentry
	VITE_SENTRY_PROJECT=         # Název projektu v Sentry
	VITE_SENTRY_AUTH_TOKEN=      # Autentizační token pro Sentry
	```
3. Sestavte aplikaci pomocí příkazu:
	```bash
	npm run build
	```
