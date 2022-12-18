# UI for the ESP32 based Tank Sensor

This new UI is build with svelte, just because I wanted to learn it! ;)

## FontAwesome Pro

This Software is build with FontAweSome Pro Icons.
In order to be able to build this package, you need a valid license and provide the auth key to npm.

```
npm config set "@fortawesome:registry" https://npm.fontawesome.com/
npm config set "//npm.fontawesome.com/:_authToken" XXXX-XXXX-XXXX-XXXX
```

## How to run locally

You need to have `npm`, `yarn`, or simmilar installed.
In addition, a up2date version of `nodejs` is required.

If you met the requirements, you can install as follows:

```
npm install
npm run dev --open
```

## How to build static files to deploy them

If you want to get some static files for your local webserver out of this code, just run:

```
npm run build
```

# License

(c) by Martin Verges.

This software is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.

You should have received a copy of the license along with this work.
If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
