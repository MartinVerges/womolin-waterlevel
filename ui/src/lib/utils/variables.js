
export const variables = {
    url: !import.meta.env.VITE_APP_URL ? `https://localhost` : import.meta.env.VITE_APP_URL,
    toast: {
        error: { theme: { '--toastBackground': '#F56565', '--toastBarBackground': '#C53030' } },
        success: { theme: { '--toastBackground': '#48BB78', '--toastBarBackground': '#2F855A' } },
    }
}
