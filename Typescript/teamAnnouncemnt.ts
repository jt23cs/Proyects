// England flag is :england: not :flag_gb-eng:
const isoMap = {
    "NETHERLANDS": "nl", "CANADA": "ca", "TURKEY": "tr", "PHILIPPINES": "ph",
    "SLOVENIA": "si", "SPAIN": "es", "SOUTH KOREA": "kr", "INDIA": "in",
    "NIGERIA": "ng", "JAPAN": "jp", "NORWAY": "no", "PORTUGAL": "pt",
    "ITALY": "it", "AUSTRALIA": "au", "BOSNIA AND HERZEGOVINA": "ba", "RUSSIA": "ru",
    "CHINA": "cn", "GERMANY": "de", "SENEGAL": "sn", "HUNGARY": "hu",
    "GREECE": "gr", "FRANCE": "fr", "UNITED STATES": "us", "CZECH REPUBLIC": "cz",
    "INDONESIA": "id", "SERBIA": "rs", "SOUTH AFRICA": "za", "NEW ZEALAND": "nz",
    "BRAZIL": "br", "CROATIA": "hr", "ENGLAND": "gb-eng", "JAMAICA": "jm"
};

const teams = await bbgm.idb.cache.teams.getAll();
const divisions = bbgm.g.get("divs");

for (const div of divisions) {
    const activeTeams = teams.filter(t => t.did === div.did && !t.disabled);

    if (activeTeams.length > 0) {
        console.log(`## __${div.name.toUpperCase()}__ ##`);

        for (const t of activeTeams) {
            const regionUpper = t.region.trim().toUpperCase();
            const iso = isoMap[regionUpper] || "unknown";
            console.log(`**${t.region} ${t.name} :flag_${iso}:**`);
        }
    }
}
