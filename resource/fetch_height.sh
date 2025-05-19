# "height": 894876,目前最高的高度

curl -s "https://blockchain.info/block-height/895465?format=json" | jq 'del(.blocks[0].tx)' > latestblock.json