# Sonoria Website

A modern, multi-lingual website for the Sonoria audio analysis library with strong SEO optimization.

## Features

- 🌍 **Multi-lingual Support**: English, Spanish, and French translations
- 🔍 **SEO Optimized**: 
  - Meta tags for search engines
  - Open Graph and Twitter Card support
  - Structured data (JSON-LD) for rich snippets
  - Semantic HTML5 markup
  - Fast loading with minimal dependencies
- 📱 **Responsive Design**: Works perfectly on all devices
- ♿ **Accessible**: WCAG compliant design patterns
- 🎨 **Modern UI**: Clean, professional design with smooth animations

## Structure

```
website/
├── index.html          # Main HTML file
├── css/
│   └── style.css       # All styles
├── js/
│   └── main.js         # Interactive functionality
├── i18n/
│   └── translations.json  # Multi-language content
└── images/             # Image assets
```

## Quick Start

### Local Development

You can serve the website locally using any static file server:

**Using Python:**
```bash
cd website
python -m http.server 8000
```

**Using Node.js:**
```bash
npx serve website
```

Then open `http://localhost:8000` in your browser.

### Deployment

The website is static and can be deployed to any hosting platform:

- **GitHub Pages**: Push to `gh-pages` branch
- **Netlify**: Drag and drop the `website` folder
- **Vercel**: Connect your repository
- **Apache/Nginx**: Copy files to your web server

## Customization

### Adding a New Language

1. Add translations to `i18n/translations.json`:
```json
{
  "de": {
    "meta": {
      "title": "Sonoria - Fortgeschrittene Audio-Analysebibliothek",
      ...
    },
    ...
  }
}
```

2. Add language button in `index.html`:
```html
<button class="lang-btn" data-lang="de">DE</button>
```

### Updating Content

All text content is stored in `i18n/translations.json`. Edit the appropriate language section to update website content.

### Styling

Modify CSS variables in `css/style.css` to change the color scheme:

```css
:root {
  --primary: #6366f1;      /* Main brand color */
  --secondary: #0ea5e9;    /* Secondary color */
  --dark: #0f172a;         /* Dark text/background */
  --light: #f8fafc;        /* Light background */
}
```

## SEO Best Practices Implemented

1. **Meta Tags**: Comprehensive meta descriptions and keywords
2. **Open Graph**: Social media sharing optimization
3. **Structured Data**: JSON-LD schema for software applications
4. **Semantic HTML**: Proper heading hierarchy and semantic elements
5. **Performance**: Minimal external dependencies, optimized CSS
6. **Mobile-First**: Responsive design for all screen sizes
7. **Accessibility**: ARIA labels and keyboard navigation

## Tutorial Examples

The website includes three practical code examples showing how to:

1. **Include the Library**: Basic setup with required headers
2. **Initialize an Algorithm**: Creating and configuring MFCC extraction
3. **Process Audio Data**: Running analysis and retrieving results

These examples use the updated `sonoria` namespace and reflect the current library API.

## License

The website content is licensed under the same license as the Sonoria library (Apache 2.0).
