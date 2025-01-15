// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

import { Client } from "@notionhq/client"
import path from "node:path"
import fs from "node:fs/promises"

const notion = new Client({ auth: process.env.NOTION_TOKEN })

const pageId = "1018c82e283f80cdab99f35ef6e34211"

// Developer toggle to test script
const doExport = true
// Log DB schema for manual run
// const db = await notion.databases.retrieve({ database_id: pageId })
// console.log(db)

let start_cursor = undefined

const dataPath = path.resolve(path.join(import.meta.dirname, '../../data/ships'))

console.log(`Writing ship JSON data files to path ${dataPath}`)

const name_to_filter = process.argv[2]

let filter_export = {
	property: "i%3Au%3E",
	checkbox: { equals: true }
}

let filter_name = {
	property: "title",
	rich_text: {
		equals: name_to_filter
	}
}

while (true) {
	let { results, next_cursor, has_more } = await notion.databases.query({
		database_id: pageId,
		start_cursor: start_cursor,
		filter: name_to_filter ? { "and": [ filter_export, filter_name ] } : filter_export,
		filter_properties: [ 'title', 'mSL%3A' /* model */,  '_R%5DR' /* JSON (Copy) */ ]
	})

	for (const result of results) {
		let name = result.properties['Name'].title[0].plain_text.toLowerCase()
		// Replace spaces and parentheses with underscores, trim trailing underscores
		name = name.replace(/[ \(\)]+/g, "_").replace(/_+$/, "")

		// FIXME: to avoid breaking savefile compatibility, we currently use the model name instead of ship name to construct the output filename
		let customName = result.properties['model'].rich_text[0]
		if (customName !== undefined) {
			name = customName.plain_text
		} else {
			console.log(`${name} is missing model field`)
		}

		const filePath = path.join(dataPath, name + '.json')
		const json = result.properties['JSON (Copy)'].formula.string + '\n'

		try {
			await fs.access(filePath)
			console.log(`Importing ship file ${filePath}`)
		} catch {
			console.log(`Ship file ${filePath} not found - first import or incorrectly-named ship?`)
		}

		if (doExport)
			await fs.writeFile(filePath, json)
	}

	start_cursor = next_cursor

	if (!has_more)
		break;
}

process.exit(0)
