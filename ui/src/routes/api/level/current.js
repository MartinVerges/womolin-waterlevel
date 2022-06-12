
export async function get({request}) {
    return {
        status: 200,
        body: Math.floor(Math.random() * 101)
    }
}