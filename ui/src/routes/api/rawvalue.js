
export async function get({request}) {
  return {
      status: 200,
      body: {
        raw: Math.floor(Math.random() * 10001)
      }
  }
}