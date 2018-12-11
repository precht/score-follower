\version "2.19.82"

#(set-default-paper-size "a6landscape")

\paper {
  indent = 0\cm
} 

\header {
  tagline = ""
}

\score {
  
  \layout {
    \context {
      \Score
      \remove "Bar_number_engraver"
    }
    \context {
      \Staff
      \remove Time_signature_engraver
      \hide BarLine
      \hide SpanBar
    }
  }
  
{
\time 32/4

