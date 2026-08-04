// Sonoria Website - Multi-language Support and Interactivity
document.addEventListener('DOMContentLoaded', function() {
  // Load translations
  let currentLang = 'en';
  let translations = {};

  // Fetch translations
  fetch('i18n/translations.json')
    .then(response => response.json())
    .then(data => {
      translations = data;
      initLanguageSwitcher();
      updateContent(currentLang);
    })
    .catch(err => console.error('Error loading translations:', err));

  // Language switcher
  function initLanguageSwitcher() {
    const langButtons = document.querySelectorAll('.lang-btn');
    langButtons.forEach(btn => {
      btn.addEventListener('click', () => {
        const lang = btn.dataset.lang;
        if (lang !== currentLang) {
          currentLang = lang;
          updateContent(lang);
          
          // Update active state
          langButtons.forEach(b => b.classList.remove('active'));
          btn.classList.add('active');
          
          // Save preference
          localStorage.setItem('sonoria_lang', lang);
        }
      });
    });

    // Check saved preference
    const savedLang = localStorage.getItem('sonoria_lang');
    if (savedLang && translations[savedLang]) {
      currentLang = savedLang;
      document.querySelector(`[data-lang="${savedLang}"]`).classList.add('active');
    }
  }

  // Update content based on language
  function updateContent(lang) {
    const t = translations[lang];
    if (!t) return;

    // Meta tags for SEO
    document.title = t.meta.title;
    updateMeta('description', t.meta.description);
    updateMeta('keywords', t.meta.keywords);

    // Navigation
    document.querySelectorAll('[data-i18n]').forEach(el => {
      const key = el.dataset.i18n;
      const keys = key.split('.');
      let value = t;
      keys.forEach(k => {
        if (value) value = value[k];
      });
      if (value) {
        if (el.tagName === 'INPUT' || el.tagName === 'TEXTAREA') {
          el.placeholder = value;
        } else {
          el.textContent = value;
        }
      }
    });

    // Features
    const featureCards = document.querySelectorAll('.feature-card');
    if (featureCards.length > 0 && t.features.items) {
      featureCards.forEach((card, index) => {
        if (t.features.items[index]) {
          card.querySelector('.feature-icon').textContent = t.features.items[index].icon;
          card.querySelector('h3').textContent = t.features.items[index].title;
          card.querySelector('p').textContent = t.features.items[index].desc;
        }
      });
    }

    // Installation tabs
    setupInstallationTabs(t);

    // Tutorial steps
    setupTutorialSteps(t);

    // Footer
    if (t.footer) {
      const copyrightEl = document.querySelector('.footer p');
      if (copyrightEl) copyrightEl.textContent = t.footer.copyright;
    }
  }

  function updateMeta(name, content) {
    let meta = document.querySelector(`meta[name="${name}"]`);
    if (!meta) {
      meta = document.createElement('meta');
      meta.name = name;
      meta.content = content;
      document.head.appendChild(meta);
    } else {
      meta.content = content;
    }
  }

  // Installation tabs functionality
  function setupInstallationTabs(t) {
    const tabButtons = document.querySelectorAll('.tab-btn');
    const codeBlock = document.querySelector('#install-code pre');
    const copyBtn = document.querySelector('.copy-btn');

    if (!t.install) return;

    tabButtons.forEach(btn => {
      btn.addEventListener('click', () => {
        // Update active state
        tabButtons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');

        // Update code
        const platform = btn.dataset.platform;
        let code = '';
        
        switch(platform) {
          case 'linux':
            code = t.install.linux_cmd;
            break;
          case 'macos':
            code = t.install.macos_cmd;
            break;
          case 'windows':
            code = t.install.windows_cmd;
            break;
          case 'source':
            code = t.install.source_cmd;
            break;
        }

        if (codeBlock) {
          codeBlock.textContent = code;
        }
      });
    });

    // Copy functionality
    if (copyBtn && codeBlock) {
      copyBtn.addEventListener('click', () => {
        navigator.clipboard.writeText(codeBlock.textContent).then(() => {
          copyBtn.textContent = 'Copied!';
          setTimeout(() => {
            copyBtn.textContent = t.install.copy || 'Copy';
          }, 2000);
        });
      });
    }
  }

  // Tutorial steps animation
  function setupTutorialSteps(t) {
    const steps = document.querySelectorAll('.step');
    
    const observer = new IntersectionObserver((entries) => {
      entries.forEach(entry => {
        if (entry.isIntersecting) {
          entry.target.style.opacity = '1';
          entry.target.style.transform = 'translateY(0)';
        }
      });
    }, { threshold: 0.1 });

    steps.forEach((step, index) => {
      step.style.opacity = '0';
      step.style.transform = 'translateY(20px)';
      step.style.transition = `all 0.6s ease ${index * 0.2}s`;
      observer.observe(step);
    });
  }

  // Smooth scrolling for navigation links
  document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function(e) {
      e.preventDefault();
      const target = document.querySelector(this.getAttribute('href'));
      if (target) {
        target.scrollIntoView({
          behavior: 'smooth',
          block: 'start'
        });
      }
    });
  });

  // Navbar scroll effect
  window.addEventListener('scroll', () => {
    const navbar = document.querySelector('.navbar');
    if (window.scrollY > 50) {
      navbar.style.boxShadow = '0 4px 6px -1px rgba(0, 0, 0, 0.1)';
    } else {
      navbar.style.boxShadow = 'none';
    }
  });
});
