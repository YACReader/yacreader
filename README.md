| Build | Master | Develop |
|---|---|---|
|Code Validation |[![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=master&jobName=CodeFormatValidation)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=master)|[![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=develop&jobName=CodeFormatValidation)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=develop)|
|Linux | [![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=master&jobName=Linux)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=master) | [![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=develop&jobName=Linux)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=develop)|
|Windows x86| [![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=master&jobName=Windows_x86)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=master)|[![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=develop&jobName=Windows_x86)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=develop)|
|Windows x64| [![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=master&jobName=Windows_x64)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=master)|[![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=develop&jobName=Windows_x64)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=develop)|
|MacOS | [![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=master&jobName=MacOS)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=master) | [![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=develop&jobName=MacOS)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=develop)|
|Publish Build||[![Build Status](https://dev.azure.com/luisangelsm/YACReader/_apis/build/status/YACReader.yacreader?branchName=develop&jobName=PublishDevBuilds)](https://dev.azure.com/luisangelsm/YACReader/_build/latest?definitionId=1&branchName=develop)|

# YACReader

"Yet another comic reader"

## License
This software has been developed by Luis Ángel San Martín Rodríguez
(luisangelsm@gmail.com) under GPL v3 license
(see [COPYING.txt](./COPYING.txt)).

## Getting YACReader

### Official releases:
[https://github.com/YACReader/yacreader/releases](https://github.com/YACReader/yacreader/releases)

### As a package:

[![Packaging status](https://repology.org/badge/vertical-allrepos/yacreader.svg)](https://repology.org/metapackage/yacreader)

### From OBS build service:

### Building from source:

See [INSTALL.md](./INSTALL.md)

## Contact
- Project home page : www.yacreader.com
- e-mail:
   - info@yacreader.com
   - support@yacreader.com
- Social:
   - Facebook  - http://www.facebook.com/YACReader
   - Twitter   - https://twitter.com/yacreader
   - YouTube   - https://www.youtube.com/user/yacreader
   - Instagram - https://www.instagram.com/yacreader/

If you need help or have any suggestion, please, send me an e-mail.

## Contributing
If you are interested in contributing to the project the first step should be to contact me so we can plan together the best approach, you can send an e-mail or just open an issue in this repo. For small bug fixes it is usually ok to open a PR directly.

Contributions are not restricted to code, you can help the project by bringing new UI/UX ideas, designing new assets, writing manuals or tutorials, translating the apps, etc. If you are interested in DevOps, YACReader uses Azure Pipelines for CI/CD, any improvements in that area are welcome. Testing pre-releases is also really appreciated.

#### Code
YACReader uses `clang-format` to ensure a common style and avoid deviances from it. CI checks this and will fail if the correct format is not used. `clang-format` needs to be called recursively in all the folders because some of them have the own `.clang-format` file, mainly to exclude changing the format in third-party libraries which are included in the source code. I recommend to configure your development tools to use `clang-format`, you can try to use it manually, but please, do it always before committing changes. I recommend using QtCreator configured properly, you can find a tutorial [here]( https://www.vikingsoftware.com/using-clang-format-with-qtcreator/).

#### CI/CD
Any PR will be validated through CI, and it will not be merged if CI fails.

#### Pull Requests
The base and target branch for any PR should always be `develop`.

## Donations
YACReader is free but it needs money to keep being alive, so please, if you like YACReader, visit the home page and make a donation.
