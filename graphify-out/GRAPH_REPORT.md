# Graph Report - H:\Ai\hermes-desktop\sherman-juce-plugin  (2026-07-06)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 43 nodes · 63 edges · 11 communities (6 shown, 5 thin omitted)
- Extraction: 98% EXTRACTED · 2% INFERRED · 0% AMBIGUOUS · INFERRED: 1 edges (avg confidence: 0.9)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_ShermanPluginAudioProcessor|ShermanPluginAudioProcessor]]
- [[_COMMUNITY_PluginProcessor.cpp|PluginProcessor.cpp]]
- [[_COMMUNITY_PluginEditor.cpp|PluginEditor.cpp]]
- [[_COMMUNITY_changeProgramName|changeProgramName]]
- [[_COMMUNITY_processBlock|processBlock]]
- [[_COMMUNITY_sherman_juce_plugin|sherman_juce_plugin]]
- [[_COMMUNITY_createPluginFilter|createPluginFilter]]
- [[_COMMUNITY_createEditor|createEditor]]
- [[_COMMUNITY_ShermanPluginAudioProcessorisBusesLayoutSupported|ShermanPluginAudioProcessor::isBusesLayoutSupported]]
- [[_COMMUNITY_getStateInformation|getStateInformation]]
- [[_COMMUNITY_JUCE_CALLTYPE createPluginFilter|JUCE_CALLTYPE createPluginFilter]]

## God Nodes (most connected - your core abstractions)
1. `ShermanPluginAudioProcessor` - 21 edges
2. `processBlock` - 4 edges
3. `createEditor` - 3 edges
4. `getName` - 3 edges
5. `getProgramName` - 3 edges
6. `changeProgramName` - 3 edges
7. `getStateInformation` - 3 edges
8. `ShermanPluginEditor::ShermanPluginEditor()` - 2 edges
9. `ShermanPluginEditor::paint()` - 2 edges
10. `ShermanPluginAudioProcessor::isBusesLayoutSupported()` - 2 edges

## Surprising Connections (you probably didn't know these)
- `ShermanPluginEditor::ShermanPluginEditor()` --references--> `ShermanPluginAudioProcessor`  [EXTRACTED]
  Source/PluginEditor.cpp → Source/PluginProcessor.h

## Import Cycles
- None detected.

## Communities (11 total, 5 thin omitted)

### Community 0 - "ShermanPluginAudioProcessor"
Cohesion: 0.25
Nodes (8): AudioProcessor, JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR, ShermanPluginAudioProcessor, acceptsMidi, getNumPrograms, hasEditor, isMidiEffect, prepareToPlay

### Community 1 - "PluginProcessor.cpp"
Cohesion: 0.25
Nodes (6): getCurrentProgram, getTailLengthSeconds, producesMidi, releaseResources, setCurrentProgram, setStateInformation

### Community 2 - "PluginEditor.cpp"
Cohesion: 0.40
Nodes (3): Graphics, ShermanPluginEditor::paint(), ShermanPluginEditor::ShermanPluginEditor()

### Community 3 - "changeProgramName"
Cohesion: 0.50
Nodes (4): changeProgramName, getName, getProgramName, String

### Community 4 - "processBlock"
Cohesion: 0.67
Nodes (3): AudioBuffer, MidiBuffer, processBlock

### Community 5 - "sherman_juce_plugin"
Cohesion: 1.00
Nodes (3): sherman_juce_plugin, ShermanPlugin, JUCE Framework

## Knowledge Gaps
- **5 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `ShermanPluginAudioProcessor` connect `ShermanPluginAudioProcessor` to `PluginProcessor.cpp`, `PluginEditor.cpp`, `changeProgramName`, `processBlock`, `createEditor`, `getStateInformation`?**
  _High betweenness centrality (0.248) - this node is a cross-community bridge._
- **Why does `processBlock` connect `processBlock` to `ShermanPluginAudioProcessor`, `PluginProcessor.cpp`?**
  _High betweenness centrality (0.081) - this node is a cross-community bridge._
- **Why does `createEditor` connect `createEditor` to `ShermanPluginAudioProcessor`, `PluginProcessor.cpp`?**
  _High betweenness centrality (0.042) - this node is a cross-community bridge._