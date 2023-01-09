// src/routes/+layout.js

// This can be false if you're using a fallback (i.e. SPA mode)
export const prerender = false;
export const ssr = false;

export const trailingSlash = 'always';

// Import our custom CSS
import '../scss/styles.scss';
