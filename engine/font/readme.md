## Design

- `data FontFilter`
    - `font_family_name: string`
    - `font_weight: FontWeight | integer`
    - `font_style: FontStyle`

- `interface FontCollection`
    - `registerFile(path: string): boolean, string`
    - `unregisterFile(path: string)`
    - `isFileRegistered(path: string): boolean`
    - `hasFontFamily(font_family_name: string): boolean`
    - `hasFont(font_family_name: string, font_weight: integer | FontWeight, font_style: FontStyle): boolean`
    - `hasFont(filter: FontFilter): boolean`
