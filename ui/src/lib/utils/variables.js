
export const variables = {
    url: !import.meta.env.VITE_APP_URL ? `https://localhost` : import.meta.env.VITE_APP_URL
}