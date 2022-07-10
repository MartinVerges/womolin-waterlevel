
export async function get({request}) {
    return {
        status: 200,
        body: {
            num: Math.round(Math.random()) + 1
        }
    }
}