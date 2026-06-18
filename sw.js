// Foki Service Worker - Phase 1 (Minimal setup)

self.addEventListener('install', (event) => {
    console.log('[Service Worker] Installed');
    // Forces the waiting service worker to become the active service worker.
    self.skipWaiting();
});

self.addEventListener('activate', (event) => {
    console.log('[Service Worker] Activated');
    // Tells the active service worker to take control of the page immediately.
    event.waitUntil(self.clients.claim());
});

self.addEventListener('fetch', (event) => {
    // Pass-through fetch (No caching yet)
    event.respondWith(fetch(event.request));
});
